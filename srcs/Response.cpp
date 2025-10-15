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

Response send_bigsize(std::map<int, Client> &clientobj, int clientFd, std::string filePath, Response &rep)
{
    if (clientobj[clientFd].Sending == 0)
    {
        clientobj[clientFd]._fd = open(filePath.c_str(), O_RDONLY);
        if (clientobj[clientFd]._fd < 0)
        {
            std::cerr << "❌ Failed to open file\n"; // or make it perror("open file")
            clientobj[clientFd].send_complete = 1;
            // and make response page of file can't be open 
            // and close connection and remove client from epoll
            // return "Failed";
        }
    struct stat filesz;
    if (fstat(clientobj[clientFd]._fd, &filesz) == -1)
    {
        perror("fstat");
        close(clientobj[clientFd]._fd);  // ✅ FIX: close opened fd on error
        clientobj[clientFd]._fd = -1;
        clientobj[clientFd].send_complete = 1;
        return rep;
    }
        clientobj[clientFd].filesize = filesz.st_size;
        clientobj[clientFd].size_send = 0;

        rep.statusCode = 200;
        // rep.contentType = "video/mp4";
        rep.filesize = clientobj[clientFd].filesize;
    }

    else
    {
        ssize_t Readbyte;
        char buffer[4000];
        Readbyte = read(clientobj[clientFd]._fd, buffer, sizeof(buffer));
        // std::cout << "###the file size is: " << clientobj[clientFd].filesize << ", and I send clientobj[clientFd].bytesRead is: " << clientobj[clientFd].bytesRead << "\n\n"; 
        if (Readbyte == -1)
        {
            // if (errno == EAGAIN || errno == EWOULDBLOCK)
            // {
            //     clientobj[clientFd].no_data = 1;
            //     return rep;
            // }
            // else
            // {
                perror("read failed");
                close(clientobj[clientFd]._fd);
                clientobj[clientFd].send_complete = 1;   
        }
        else if (Readbyte == 0)
        {
            rep.body.assign(buffer, Readbyte);
            clientobj[clientFd].bytesRead = Readbyte;
            // std::cout << "the only explanation is to see this message\n";
            clientobj[clientFd].send_complete = 1;
            close(clientobj[clientFd]._fd);
        }
        else if (Readbyte > 0)
        {
            rep.body.assign(buffer, Readbyte);
            clientobj[clientFd].bytesRead = Readbyte;
        }
        else
        {
            close(clientobj[clientFd]._fd);
            perror("read failed");
        }
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
        send(clientFd, response.c_str(), response.size(), MSG_NOSIGNAL);

        std::cout << "✅ Redirect response sent to FD: " << clientFd << " with the return code is: " << statusStr << std::endl;
        clientobj[clientFd].send_complete = 1;
    }

    else if (clientobj[clientFd].method == "GET" && clientobj[clientFd].has_cgi && clientobj[clientFd].Sending == 0 && clientobj[clientFd].statusCode != 204 && 
        clientobj[clientFd].statusCode != 504 && clientobj[clientFd].statusCode != 408)
    {
        if(clientobj[clientFd].has_cookie == 0)  //zadt cookies
        {
            srand(time(NULL));
            clientobj[clientFd].sessionId = generateId(16);
            clientobj[clientFd].getSession().push_back(clientobj[clientFd].sessionId);
            std::cout << "Set-Cookie: session_id=" << clientobj[clientFd].sessionId << "\n";
            std::cout << "Hello, new user! Data saved on server.\n\n";
        }
        std::ostringstream heaaad;
        heaaad << "HTTP/1.1 " << clientobj[clientFd].statusCode << "\r\n"
            << "Content-Type: " << clientobj[clientFd].ContentType << "\r\n";
        if(clientobj[clientFd].has_cookie == 0)  //zadt cookies
        {
            heaaad<< "Set-Cookie: session_id=" << clientobj[clientFd].sessionId << "\r\n";
            clientobj[clientFd].has_cookie = 1;
        }
            heaaad << "Connection: keep-alive\r\n\r\n";

        std::string headerStr = heaaad.str();
        // std::cout << "the headersssssss sennnnnding: " << headerStr << std::endl;
        // std::cout << "**************booody  is: " <<  clientobj[clientFd].CgiBody << std::endl;
        send(clientFd, headerStr.c_str(), headerStr.size(), MSG_NOSIGNAL);
        clientobj[clientFd].Sending = 1;
    }
    else if (!clientobj[clientFd].has_cgi && clientobj[clientFd].method == "GET" && clientobj[clientFd].Sending == 0
        && !clientobj[clientFd].ResponseChunked)
    {
        // std::cout << "coming to this condition is acceptable and true\n\n";
        std::ostringstream headers;
        headers << "HTTP/1.1 " << res.statusCode << "\r\n"
            << "Content-Type: " << res.contentType << "\r\n";
        if(clientobj[clientFd].has_cookie == 0)  //zadt cookies
        {
            headers<< "Set-Cookie: session_id=" << res.sessionId << "\r\n";
            clientobj[clientFd].has_cookie = 1;
        }
            headers<< "Content-Length: " << res.filesize << "\r\n"
            << "Connection: : keep-alive\r\n\r\n";

        std::string headerStr = headers.str();
        send(clientFd, headerStr.c_str(), headerStr.size(), MSG_NOSIGNAL);
        clientobj[clientFd].Sending = 1;
    }
    else if ((clientobj[clientFd].method == "GET" && clientobj[clientFd].has_cgi && clientobj[clientFd].Sending == 1
        && !clientobj[clientFd].send_complete && clientobj[clientFd].statusCode != 204) || 
        (clientobj[clientFd].method == "GET" && clientobj[clientFd].has_cgi && clientobj[clientFd].Sending == 1
        && clientobj[clientFd].send_complete && clientobj[clientFd].statusCode != 204 && !clientobj[clientFd].has_problem))
    {
        std::string chunkmybody;
        if (clientobj[clientFd].CgiBody.size() > 8000)
        {
            chunkmybody = clientobj[clientFd].CgiBody.substr(0, 8000);
            clientobj[clientFd].CgiBody = clientobj[clientFd].CgiBody.substr(8000);
        }
        else
        {
            chunkmybody = clientobj[clientFd].CgiBody;
            clientobj[clientFd].CgiBody = "";
            clientobj[clientFd].send_complete = 1;
        }
        // std::cout << "**************chunkmybody id: " << chunkmybody << std::endl;
        ssize_t sendbytes = send(clientFd, chunkmybody.c_str(), chunkmybody.size(), MSG_NOSIGNAL);
        if (sendbytes != -1)
            clientobj[clientFd].size_send += sendbytes;
    }
    else if (!clientobj[clientFd].has_cgi && clientobj[clientFd].method == "GET" && clientobj[clientFd].Sending == 1
        && !clientobj[clientFd].ResponseChunked)
    {
        ssize_t sendbytes = send(clientFd, res.body.c_str(), clientobj[clientFd].bytesRead, MSG_NOSIGNAL);
        if (sendbytes != -1)
            clientobj[clientFd].size_send += sendbytes;
    }
    else if (((!clientobj[clientFd].has_cgi || clientobj[clientFd].method == "POST") && clientobj[clientFd].ResponseChunked) || 
        clientobj[clientFd].method.empty())
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
        
        std::cout << " haniiiiiiiiiiiiiii\n\n" << " codeeeeeeeeeee "<< clientobj[clientFd].response.statusCode<<std::endl;
        sent = send(clientFd, response.str().c_str(), response.str().size(), MSG_NOSIGNAL);
    }
    else if (clientobj[clientFd].statusCode == 204)
    {
        // std::cout << "this is no content 204 cgi is; " << "\n";
        // std::cout << "and body is :" << clientobj[clientFd].response.body << "<=---\n\n";
        response << "HTTP/1.0 " << clientobj[clientFd].response.statusCode << "\r\n"
                << "Content-Type: " << clientobj[clientFd].response.contentType << "\r\n";
                if(clientobj[clientFd].has_cookie == 0)  //zadt cookies
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
        std::cout << "hooooona has no cgi is; the conditions are: has_cgi is==> " << clientobj[clientFd].has_cgi << "and sending var is==> " << clientobj[clientFd].Sending
        << "and response chunked var is==>" << clientobj[clientFd].ResponseChunked << "and lastly send_complete var is==>" << clientobj[clientFd].send_complete << "\n\n\n";
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
        // if (errno == EPIPE)
        // { // do nothing or debugging msg because the connection is closed:) 
        //     std::cout << "❌client closed connection❌\n\n";
        //     close(clientobj[clientFd]._fd);
        //         // or maybe remove client from epoll events or epoll fds
        // }
        // if (errno == EAGAIN || errno == EWOULDBLOCK)
        //     return;
        // else
            std::cerr << "❌ send failed: " << strerror(errno) << std::endl;
        //maybe we should close the connection if send failed
    }
    // else
    // {
    //     std::cout << "✅ File response sent to FD: " << clientFd << std::endl;
    //     // return ;
    // }
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
    std::cout << " code : "<< rep.statusCode<<std::endl; 
    std::ifstream file(filePath.c_str(), std::ios::in | std::ios::binary);
    // std::cout << " fil repone %%%%%%% " << filePath << std::endl;
    if (!file)
    {
        std::cerr << "❌❌ Failed to open file: " << filePath << std::endl;
        rep.statusCode = 500;
        rep.statusMsg  = "Internal Server Error";
        rep.body = "<!DOCTYPE html>\n"
           "<html lang=\"en\">\n"
           "<head>\n"
           "    <meta charset=\"UTF-8\">\n"
           "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
           "    <title>500 Internal Server Error</title>\n"
           "    <style>\n"
           "        body {\n"
           "            font-family: Arial, sans-serif;\n"
           "            background-color: #f2f2f2;\n"
           "            color: #333;\n"
           "            text-align: center;\n"
           "            padding: 50px;\n"
           "        }\n"
           "        h1 {\n"
           "            color: #ff0000;\n"
           "            font-size: 72px;\n"
           "        }\n"
           "        p {\n"
           "            font-size: 24px;\n"
           "        }\n"
           "    </style>\n"
           "</head>\n"
           "<body>\n"
           "    <h1>500</h1>\n"
           "    <p>Internal Server Error</p>\n"
           "    <p>Something went wrong on our server. Please try again later.</p>\n"
           "</body>\n"
           "</html>\n";
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
            rep.contentType = "image/jpeg";
        else if (filePath.find(".png") != std::string::npos)
            rep.contentType = "image/png";
        else if (filePath.find(".mp4") != std::string::npos)
            rep.contentType = "video/mp4";
        if (filePath.find(".mp4") != std::string::npos || filePath.find(".png") != std::string::npos ||
            filePath.find(".jpg") != std::string::npos || filePath.find(".jpeg") != std::string::npos)
        {
            clientobj[clientFd].ResponseChunked = 0;
            send_bigsize(clientobj, clientFd, filePath, rep);
        }
    }
    if ((clientobj[clientFd].method == "GET" && clientobj[clientFd].ResponseChunked == 1 && !clientobj[clientFd].autoindex) || clientobj[clientFd].method != "GET" || clientobj[clientFd].has_problem)
    {
         std::cout << "helllllllllllllo------------------>>>>>>>>"<<clientobj[clientFd].method<<std::endl;
        std::ostringstream ss; // to put file content in it ;)
        ss << file.rdbuf();
        rep.body = ss.str();
        //std::cout << "THE BODY OF UR FILE IS: " << rep.body << std::endl;
    }
    return rep;
}
