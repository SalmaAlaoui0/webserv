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
            clientobj[clientFd].send_complete = 1;
            // and make response page of file can't be open 
            // and close connection and remove client from epoll
            // return "Failed";
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
        char buffer[8000];
        Readbyte = read(clientobj[clientFd]._fd, buffer, sizeof(buffer));


        if (Readbyte == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                clientobj[clientFd].no_data = 1;
                return rep;
            }
            else
            {
                clientobj[clientFd].send_complete = 1;
                perror("read failed");
            }
        }
        else if (Readbyte == 0)
        {
            rep.body.assign(buffer, Readbyte);
            rep.Readbyte = Readbyte;
            clientobj[clientFd].send_complete = 1;
            close(clientobj[clientFd]._fd);
        }
        else if (Readbyte > 0)
        {
            rep.body.assign(buffer, Readbyte);
            rep.Readbyte = Readbyte;
        }
        else
        {
            perror("read failed");
        }
    }
    return rep;
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
//std::cout << " wafaaaa%%%%%%%%%% "<< clientobj[clientFd].statusCode<< std::endl;
    if (clientobj[clientFd].statusCode == 302)
    {
        std::string body =
            "<html>\n"
            "<head><title>302 Found</title></head>\n"
            "<body>\n"
            "<p>Resource moved to <a href=\"" + clientobj[clientFd].ReturnLocation + "\">" + clientobj[clientFd].ReturnLocation + "</a></p>\n"
            "</body>\n"
            "</html>";

        std::ostringstream headers;
        headers << "HTTP/1.1 302 Found\r\n"
                << "Location: " << clientobj[clientFd].ReturnLocation << "\r\n"
                << "Content-Type: text/html\r\n"
                << "Content-Length: " << body.size() << "\r\n"
                << "Connection: close\r\n\r\n"
                << body;

        std::string response = headers.str();
        send(clientFd, response.c_str(), response.size(), MSG_NOSIGNAL);

        std::cout << "it's 302 \n";
        std::cout << "the data location should be served is: " << clientobj[clientFd].ReturnLocation << std::endl;
    }
    else if (clientobj[clientFd].method == "GET" && clientobj[clientFd].has_cgi && clientobj[clientFd].Sending == 0 && clientobj[clientFd].Read)
    {
        if(clientobj[clientFd].has_cookie == 0)  //zadt cookies
        {
            srand(time(NULL));
            clientobj[clientFd].sessionId = generateId(16);
            clientobj[clientFd].getSession().push_back(clientobj[clientFd].sessionId);
            std::cout << "Set-Cookie: session_id=" << clientobj[clientFd].sessionId << "\n";
            std::cout << "Hello, new user! Data saved on server.\n\n";
        }
        clientobj[clientFd].ContentType = "text/html";
        size_t HeaderEnd = clientobj[clientFd].CgiBody.find("\r\n\r\n");
        size_t sepLength = 4; // default CRLF

        if (HeaderEnd == std::string::npos) {
            HeaderEnd = clientobj[clientFd].CgiBody.find("\n\n");
            sepLength = 2; // LF only
        }

        if (HeaderEnd != std::string::npos)
        {
            std::string headers = clientobj[clientFd].CgiBody.substr(0, HeaderEnd);
            clientobj[clientFd].ContentType = ft_content_type(headers);
            clientobj[clientFd].statusCode = ft_code_status(headers);
            clientobj[clientFd].CgiBody = clientobj[clientFd].CgiBody.substr(HeaderEnd + sepLength);
        }
        if (clientobj[clientFd].ContentType == "video/mp4" || clientobj[clientFd].ContentType == "image/png" || 
            clientobj[clientFd].ContentType == "image/jpg" || clientobj[clientFd].ContentType == "image/jpeg")
            clientobj[clientFd].Read = 0;
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
        send(clientFd, headerStr.c_str(), headerStr.size(), MSG_NOSIGNAL);
        clientobj[clientFd].Sending = 1;
    }
    else if (!clientobj[clientFd].has_cgi && clientobj[clientFd].method == "GET" && clientobj[clientFd].Sending == 0
        && !clientobj[clientFd].ResponseChunked)
    {
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
    else if (clientobj[clientFd].method == "GET" && clientobj[clientFd].has_cgi && clientobj[clientFd].Sending == 1
        && !clientobj[clientFd].send_complete && clientobj[clientFd].Read)
    {
        ssize_t sendbytes = send(clientFd, clientobj[clientFd].CgiBody.c_str(), clientobj[clientFd].CgiBody.size(), MSG_NOSIGNAL);
        if (sendbytes != -1)
        {
            clientobj[clientFd].Read = 0;
            clientobj[clientFd].size_send += sendbytes;
        }
    }
    else if (!clientobj[clientFd].has_cgi && clientobj[clientFd].method == "GET" && clientobj[clientFd].Sending == 1
        && !clientobj[clientFd].ResponseChunked && !clientobj[clientFd].send_complete)
    {
        ssize_t sendbytes = send(clientFd, res.body.c_str(), Readbyte, MSG_NOSIGNAL);
        if (sendbytes != -1)
            clientobj[clientFd].size_send += sendbytes;
    }
    else if (((!clientobj[clientFd].has_cgi || clientobj[clientFd].method == "POST") && clientobj[clientFd].ResponseChunked) || 
        clientobj[clientFd].method.empty())
    {
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
        return;
    }
    if (sent < 0)
    {
        if (errno == EPIPE)
        { // do nothing or debugging msg because the connection is closed:) 
            std::cout << "❌client closed connection❌\n\n";
            close(clientobj[clientFd]._fd);
                // or maybe remove client from epoll events or epoll fds
        }
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return;
        else
            std::cerr << "❌ send failed: " << strerror(errno) << std::endl;
        //maybe we should close the connection if send failed
    }
    else
    {
        // std::cout << "✅ File response sent to FD: " << clientFd << std::endl;
        return ;
    }
    return;
}

Response Response::buildResponse(request &r, int code, const std::string &msg, const std::string &filePath, int clientFd, std::map<int, Client> &clientobj)
{
    std::cout << " i m in errorr setttttttttttttt\n\n";

    Response rep;
    Server s;
    if(clientobj[clientFd].has_cookie == 0)  //zadt cookies
    {
        srand(time(NULL));
        rep.sessionId = generateId(16);
        s.getSession() .push_back(rep.sessionId);
        std::cout << "Set-Cookie: session_id=" << rep.sessionId << "\n";
        std::cout << "Hello, new user! Data saved on server.\n\n";
        // exit(7); if I have a cgi script
    }
    if (code == 400 && filePath.empty())
    {
        rep.statusCode = 400;
        rep.statusMsg  = msg;
        rep.body       = "<h1>400 Bad Request</h1>";
        rep.contentType = "text/html";
        clientobj[clientFd].Sending = 1;
        clientobj[clientFd].send_complete = 1;
        return (rep);
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
    rep.statusMsg = msg;
    std::ifstream file(filePath.c_str(), std::ios::in | std::ios::binary);
    if (!file)
    {
        std::cerr << "❌❌ Failed to open file: " << filePath << std::endl;
        rep.statusCode = 404;
        rep.statusMsg  = "Not Found";
        rep.body       = "<h1>404 Not Found</h1>";
        rep.contentType = "text/html";
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
    if ((r.get_method() == "GET" && clientobj[clientFd].ResponseChunked == 1 && !clientobj[clientFd].autoindex) || r.get_method() != "GET")
    {
        // std::cout << "helllllllllllllo\n";
        std::ostringstream ss; // to put file content in it ;)
        ss << file.rdbuf();
        rep.body = ss.str();
        // std::cout << "THE BODY OF UR FILE IS: " << rep.body << std::endl;
    }
    return rep;
}
