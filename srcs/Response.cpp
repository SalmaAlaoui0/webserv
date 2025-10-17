#include "../includes/Server.hpp"
#include "../includes/Response.hpp"
#include "../includes/Request.hpp"
#include "../includes/Utils.hpp"
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

Response::Response() : statusCode(0), statusMsg(""), body(""), contentType("text/html") {}

Response::~Response() {}

Response send_bigsize(std::map<int, Client> &clientobj, int clientFd, std::string filePath, Response &rep, std::vector<ServerConfig> &_configs)
{
    (void)_configs;
    if (clientobj[clientFd].Sending == 0)
    {
        if (!clientobj[clientFd].FileOpened)
        {
            clientobj[clientFd]._fd = open(filePath.c_str(), O_RDONLY);
            clientobj[clientFd].FileOpened = 1;
            std::cout << "🎉Opened fd: " << clientobj[clientFd]._fd << std::endl;
        }
        if (clientobj[clientFd]._fd < 0)
        {
            std::cerr << "❌ Failed to open file\n";
            clientobj[clientFd].send_complete = 1;
        }
        struct stat filesz;
        if (fstat(clientobj[clientFd]._fd, &filesz) == -1)
        {
            std::cerr << "❌ fstat failed\n";
            close(clientobj[clientFd]._fd);
            clientobj[clientFd]._fd = -1;
            clientobj[clientFd].send_complete = 1;
            return rep;
        }
        clientobj[clientFd].filesize = filesz.st_size;
        clientobj[clientFd].size_send = 0;

        rep.statusCode = 200;
        rep.filesize = clientobj[clientFd].filesize;
    }

    else
    {
        ssize_t Readbyte;
        char buffer[4000];
        Readbyte = read(clientobj[clientFd]._fd, buffer, sizeof(buffer));
        if (Readbyte == -1)
        {
            close(clientobj[clientFd]._fd);
            clientobj[clientFd].send_complete = 1;
        }
        else if (Readbyte == 0)
        {
            rep.body.assign(buffer, Readbyte);
            clientobj[clientFd].bytesRead = Readbyte;
            clientobj[clientFd].send_complete = 1;
            close(clientobj[clientFd]._fd);
        }
        else if (Readbyte > 0)
        {
            rep.body.assign(buffer, Readbyte);
            clientobj[clientFd].bytesRead = Readbyte;
        }
        else
            close(clientobj[clientFd]._fd);
    }
    return rep;
}

static std::string intToString(int num)
{
    std::ostringstream ss;
    ss << num;
    return ss.str();
}

std::string generateId(size_t length = 16) 
{
    const char set[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    std::string result;
    result.reserve(length);

    for (size_t i = 0; i < length; i++)
        result += set[rand() % (sizeof(set) - 1)];
    return result;
}

void Response::RequestResponse(int clientFd, Response &res, std::map<int, Client> &clientobj)
{
    std::ostringstream response;
    ssize_t sent = 0;

    if (clientobj[clientFd].statusCode == 301 || clientobj[clientFd].statusCode == 302)
    {
        std::string statusStr = intToString(clientobj[clientFd].statusCode); 
        std::string reasonPhrase = (clientobj[clientFd].statusCode == 301)
                                    ? "Moved Permanently"
                                    : "Found";

        std::string body =
            "<html>\n"
            "<head><title>" + statusStr + " " + reasonPhrase + "</title></head>\n"
            "<body>\n"
            "<p>Resource moved to <a href=\"" + clientobj[clientFd].ReturnLocation + "\">" +
            clientobj[clientFd].ReturnLocation + "</a></p>\n"
            "</body>\n"
            "</html>";

        std::ostringstream headers;
        headers << "HTTP/1.1 " << clientobj[clientFd].statusCode << " " << reasonPhrase << "\r\n"
                << "Location: " << clientobj[clientFd].ReturnLocation << "\r\n"
                << "Content-Type: text/html\r\n"
                << "Content-Length: " << body.size() << "\r\n"
                << "Connection: close\r\n\r\n"
                << body;

        std::string response = headers.str();
        sent = send(clientFd, response.c_str(), response.size(), MSG_NOSIGNAL);
        clientobj[clientFd].send_complete = 1;
    }

    else if ((clientobj[clientFd].method == "GET" && clientobj[clientFd].has_cgi && clientobj[clientFd].Sending == 0 &&
        clientobj[clientFd].statusCode != 204 && clientobj[clientFd].statusCode != 504 && clientobj[clientFd].statusCode != 408) ||
        (!clientobj[clientFd].has_cgi && clientobj[clientFd].method == "GET" && clientobj[clientFd].Sending == 0
        && !clientobj[clientFd].ResponseChunked))
    {
        if(clientobj[clientFd].has_cookie == 0)
        {
            srand(time(NULL));
            clientobj[clientFd].sessionId = generateId(16);
            clientobj[clientFd].getSession().push_back(clientobj[clientFd].sessionId);
            std::cout << "Set-Cookie: session_id=" << clientobj[clientFd].sessionId << "\n";
            std::cout << "Hello, new user! Data saved on server.\n\n";
        }
        std::ostringstream headers;
        headers << "HTTP/1.1 " << clientobj[clientFd].statusCode << "\r\n"
            << "Content-Type: " << clientobj[clientFd].ContentType << "\r\n";
        if(clientobj[clientFd].has_cookie == 0)
        {
            headers<< "Set-Cookie: session_id=" << clientobj[clientFd].sessionId << "\r\n";
            clientobj[clientFd].has_cookie = 1;
        }
        if (!clientobj[clientFd].has_cgi)
            headers<< "Content-Length: " << res.filesize << "\r\n";

        headers << "Connection: keep-alive\r\n\r\n";

        std::string headerStr = headers.str();
        sent = send(clientFd, headerStr.c_str(), headerStr.size(), MSG_NOSIGNAL);
        clientobj[clientFd].Sending = 1;
    }
    else if ((clientobj[clientFd].method == "GET" && clientobj[clientFd].has_cgi && clientobj[clientFd].Sending == 1
        && !clientobj[clientFd].send_complete && clientobj[clientFd].statusCode != 204) || 
        (clientobj[clientFd].method == "GET" && clientobj[clientFd].has_cgi && clientobj[clientFd].Sending == 1
        && clientobj[clientFd].send_complete && clientobj[clientFd].statusCode != 204 && !clientobj[clientFd].has_problem))
    {
        std::string chunkmybody;
        if (clientobj[clientFd].CgiBody.size() > 12000)
        {
            chunkmybody = clientobj[clientFd].CgiBody.substr(0, 12000);
            clientobj[clientFd].CgiBody = clientobj[clientFd].CgiBody.substr(12000);
        }
        else
        {
            chunkmybody = clientobj[clientFd].CgiBody;
            clientobj[clientFd].CgiBody = "";
            clientobj[clientFd].send_complete = 1;
        }
        sent = send(clientFd, chunkmybody.c_str(), chunkmybody.size(), MSG_NOSIGNAL);
        if (sent > -1)
            clientobj[clientFd].size_send += sent;
    }
    else if (!clientobj[clientFd].has_cgi && clientobj[clientFd].method == "GET" && clientobj[clientFd].Sending == 1
        && !clientobj[clientFd].ResponseChunked && !clientobj[clientFd].has_problem)
    {
        clientobj[clientFd].updateActivity();
        ssize_t sendbytes = send(clientFd, res.body.c_str(), clientobj[clientFd].bytesRead, MSG_NOSIGNAL);
        if (sendbytes != -1)
            clientobj[clientFd].size_send += sendbytes;
    }
    else if (((!clientobj[clientFd].has_cgi || clientobj[clientFd].method == "POST") && clientobj[clientFd].ResponseChunked) || 
        (clientobj[clientFd].method.empty()) || (clientobj[clientFd].statusCode == 204))
    {
        response << "HTTP/1.1 " << clientobj[clientFd].response.statusCode << "\r\n"
        << "Content-Type: " << clientobj[clientFd].response.contentType << "\r\n";
        if(clientobj[clientFd].has_cookie == 0)
        {
            response << "Set-Cookie: session_id=" << clientobj[clientFd].response.sessionId <<"\r\n";
            clientobj[clientFd].has_cookie = 1;
        }
        response << "Content-Length: " << clientobj[clientFd].response.body.size() << "\r\n\r\n"
        << clientobj[clientFd].response.body;
        sent = send(clientFd, response.str().c_str(), response.str().size(), MSG_NOSIGNAL);
    }
    else
    {
         response << "HTTP/1.1 " << clientobj[clientFd].response.statusCode << "\r\n"
                << "Content-Type: " << clientobj[clientFd].response.contentType << "\r\n";
                if(clientobj[clientFd].has_cookie == 0)
                {
                    response << "Set-Cookie: session_id=" << clientobj[clientFd].response.sessionId <<"\r\n";
                    clientobj[clientFd].has_cookie = 1;
                }
                response << "Content-Length: " << clientobj[clientFd].response.body.size() << "\r\n\r\n"
                << clientobj[clientFd].response.body;

        sent = send(clientFd, response.str().c_str(), response.str().size(), MSG_NOSIGNAL);
        return;
    }
    if (sent < 0)
    {
        std::cerr << "❌ send failed: " << strerror(errno) << std::endl;
        clientobj[clientFd].send_complete = 1;
    }
    if (sent == 0)
        return;
    return;
}

Response Response::buildResponse(int code, const std::string msg, std::string filePath, int clientFd, std::map<int, Client> &clientobj, std::vector<ServerConfig> &_configs)
{
    Response rep;
    Server s;
    if (code == 400 && filePath.empty())
    {
        rep.statusCode = 400;
        rep.statusMsg  = "Bad Request";
        filePath      = _configs[clientobj[clientFd]. conf_i].ErrorPages[400];
        rep.contentType = "text/html";
        clientobj[clientFd].Sending = 1;
        clientobj[clientFd].send_complete = 1;
        return (rep);
    }
    if(clientobj[clientFd].has_cookie == 0)
    {
        srand(time(NULL));
        rep.sessionId = generateId(16);
        s.getSession() .push_back(rep.sessionId);
        std::cout << "Set-Cookie: session_id=" << rep.sessionId << "\n";
        std::cout << "Hello, new user! Data saved on server.\n\n";
    }
    if (clientobj[clientFd].autoindex)
    {
        rep.contentType = "text/html";
        rep.statusCode = 200;
        rep.statusMsg = "OK";
        rep.body = clientobj[clientFd].autoIndexBody;
        return rep;
    }
    rep.statusCode = code;
    clientobj[clientFd].statusCode = code;
    rep.statusMsg = msg;
    clientobj[clientFd].statusMsg = msg;
    std::ifstream file(filePath.c_str(), std::ios::in | std::ios::binary);
    if (!file && clientobj[clientFd].statusCode == 200)
    {
        clientobj[clientFd].statusCode = 403;
        clientobj[clientFd].statusMsg = "Forbidden";
        rep.statusCode = 403;
        rep.statusMsg = "Forbidden";
        std::ifstream file(_configs[clientobj[clientFd].conf_i].ErrorPages[403].c_str(), std::ios::in | std::ios::binary);
    }
    if (!file)
    {
        std::string statusStr = intToString(clientobj[clientFd].statusCode);
        std::string reasonPhrase = clientobj[clientFd].statusMsg;

        rep.body =
            "<html>\n"
            "<head><title>" + statusStr + "</title></head>\n"
            "<body>\n"
            "<h1>" + statusStr + " " + reasonPhrase + "</h1>\n"
            "</body>\n"
            "</html>";

        // filePath      = _configs[clientobj[clientFd]. conf_i].ErrorPages[500];
        rep.contentType = "text/html";
        clientobj[clientFd].has_problem = 1;
        clientobj[clientFd].Sending = 1;
        clientobj[clientFd].send_complete = 1;
        return (rep);
    }
    else
    {
        clientobj[clientFd].ResponseChunked = 1;
        rep.contentType = "text/plain";
        if (filePath.find(".html") != std::string::npos)
            rep.contentType = "text/html";
        else if (filePath.find(".css") != std::string::npos)
            rep.contentType = "text/css";
        else if (filePath.find(".js") != std::string::npos)
            rep.contentType = "application/javascript";
        else if (filePath.find(".jpg") != std::string::npos || filePath.find(".jpeg") != std::string::npos)
            clientobj[clientFd].ContentType = "image/jpeg";
        else if (filePath.find(".png") != std::string::npos)
            clientobj[clientFd].ContentType = "image/png";
        else if (filePath.find(".mp4") != std::string::npos)
            clientobj[clientFd].ContentType = "video/mp4";
        if (filePath.find(".mp4") != std::string::npos || filePath.find(".png") != std::string::npos ||
            filePath.find(".jpg") != std::string::npos || filePath.find(".jpeg") != std::string::npos)
        {
            clientobj[clientFd].ResponseChunked = 0;
            send_bigsize(clientobj, clientFd, filePath, rep, _configs);
        }
    }
    if ((clientobj[clientFd].method == "GET" && clientobj[clientFd].ResponseChunked == 1 && !clientobj[clientFd].autoindex) || clientobj[clientFd].method != "GET" || clientobj[clientFd].has_problem)
    {
        std::ostringstream ss;
        ss << file.rdbuf();
        rep.body = ss.str();
    }
    return rep;
}
