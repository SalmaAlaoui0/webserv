#include "../includes/Server.hpp"
#include "../includes/Response.hpp"
#include "../includes/Request.hpp"
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>


Response::Response() : statusCode(0), statusMsg(""), body(""), contentType("text/html") {}

Response::~Response() {}

Response send_video(std::map<int, Client> &clientobj, int clientFd, std::string filePath, Response &rep)
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
        rep.contentType = "video/mp4";
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
        if (Readbyte == 8000)
        {
            rep.body.assign(buffer, Readbyte);
            rep.Readbyte = Readbyte;
        }
        else if (Readbyte < 8000)
        {
            rep.body.assign(buffer, Readbyte);
            rep.Readbyte = Readbyte;
            std::cout << "✅ Reading has been finished" << std::endl;
            clientobj[clientFd].send_complete = 1;
            close(clientobj[clientFd]._fd);
        }
        else
        {
            perror("read failed");
        }
    }
    return rep;
}
void Response::RequestResponse(int clientFd, Response &res, std::map<int, Client> &clientobj)
{
    std::ostringstream response;
    ssize_t sent = 0;
    std::cout << clientobj[clientFd].method << "<==== this is the method\n\n";
    if (clientobj[clientFd].method == "GET" && clientobj[clientFd].Sending == 0
        && !clientobj[clientFd].ResponseChunked)
    {
        std::ostringstream headers;
        headers << "HTTP/1.1 " << res.statusCode << "\r\n"
            << "Content-Type: " << res.contentType << "\r\n"
            << "Content-Length: " << res.filesize << "\r\n"
            << "Connection: : keep-alive\r\n\r\n";

        std::string headerStr = headers.str();
        std::cout << "here1\n";// keep those u'll need them 
        send(clientFd, headerStr.c_str(), headerStr.size(), MSG_NOSIGNAL);
        clientobj[clientFd].Sending = 1;
    }
    else if (clientobj[clientFd].method == "GET" && clientobj[clientFd].Sending == 1
        && !clientobj[clientFd].ResponseChunked)
    {
        // std::cout << "ur buffer size is: " << res.body.size() << " and readbyte are: " << Readbyte << "<----\n\n";
        std::cout << "here2\n";// keep those u'll need them 
        ssize_t sendbytes = send(clientFd, res.body.c_str(), Readbyte, MSG_NOSIGNAL);
        if (sendbytes != -1)
        {
            clientobj[clientFd].size_send += sendbytes;
            std::cout << "byte sended are : " << clientobj[clientFd].size_send << "and file size is: " << clientobj[clientFd].filesize << std::endl;
        }
    }
    // and send response
    // 2-if method is "GET" and first respose than send header of get path
    // 3-if method is "GET" and not first time than send only a body coming in res.body
    else
    {
        response << "HTTP/1.0 " << clientobj[clientFd].response.statusCode << "\r\n"
                << "Content-Type: " << clientobj[clientFd].response.contentType << "\r\n"
                << "Content-Length: " << clientobj[clientFd].response.body.size() << "\r\n\r\n"
                << res.body;

        std::cout << "here3\n";// keep those u'll need them 
        sent = send(clientFd, response.str().c_str(), response.str().size(), 0);
    }
    if (sent < 0)
    {
        if (errno == EPIPE)
        { // do nothing or debugging msg because the connection is closed:) 
            std::cout << "❌client closed connection❌\n\n";
            close(clientobj[clientFd]._fd);
                // or maybe remove client from epoll events or epoll fds
        }
        else
            std::cerr << "❌ send failed: " << strerror(errno) << std::endl;
        //maybe we should close the connection if send failed
    }
    else
    {
        std::cout << "✅ File response sent to FD: " << clientFd << std::endl;
        return ;
    }
    return;
}

Response Response::buildResponse(request &r, int code, const std::string &msg, const std::string &filePath, int clientFd, std::map<int, Client> &clientobj)
{
    std::cerr << "\n@@@@@@@Trying to open: [" << filePath << "]\n";

    Response rep;
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
        std::cerr << "❌ Failed to open file: " << filePath << std::endl;
        rep.statusCode = 404;
        rep.statusMsg  = "Not Found";
        rep.body       = "<h1>404 Not Found</h1>";
        rep.contentType = "text/html";
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
        {
            clientobj[clientFd].ResponseChunked = 0;
            send_video(clientobj, clientFd, filePath, rep);
        }
        std::cout << "data are==>: the file path is: " << filePath << " and the type is: " << rep.contentType << "\n ";
        exit(43);
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
