#include "../includes/Server.hpp"
#include "../includes/Response.hpp"
#include "../includes/Request.hpp"
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>


Response::Response() : statusCode(0), statusMsg(""), body(""), contentType("text/html") {}

Response::~Response() {}

std::string send_video(std::map<int, Client> &clientobj, int clientFd, std::string filePath, std::string resCode)
{
    if (clientobj[clientFd].Sending == 0)
    {
        clientobj[clientFd]._fd = open(filePath.c_str(), O_RDONLY);
        if (clientobj[clientFd]._fd < 0)
        {
            std::cerr << "❌ Failed to open file\n"; // or make it perror("open file")
            return "Failed";
        }
        struct stat filesz;
        if (fstat(clientobj[clientFd]._fd, &filesz) == -1)
        {
            perror("fstat");
            return "Failed";
        }
        clientobj[clientFd].filesize = filesz.st_size;
        clientobj[clientFd].size_send = 0;
        

        std::ostringstream headers;
        headers << "HTTP/1.1 " << resCode << "\r\n"
                << "Content-Type: " << "video/mp4" << "\r\n"
                << "Content-Length: " << clientobj[clientFd].filesize << "\r\n"
                << "Connection: : keep-alive\r\n\r\n";

        std::string headerStr = headers.str();
        send(clientFd, headerStr.c_str(), headerStr.size(), MSG_NOSIGNAL);
        clientobj[clientFd].Sending = 1;
    }

    size_t Readbyte;
    char buffer[8000];
    Readbyte = read(clientobj[clientFd]._fd, buffer, sizeof(buffer));

    if (Readbyte > 0)
    {
        ssize_t sendbytes = send(clientFd, buffer, Readbyte, MSG_NOSIGNAL);
        if (sendbytes == -1)
        {
            if (errno == EPIPE){ // do nothing or debugging msg because the connection is closed:) 
                }            
            else
                perror("send failed");
        }
        else
        {
            clientobj[clientFd].size_send += sendbytes;
            std::cout << "byte sended are : " << clientobj[clientFd].size_send << "and file size is: " << clientobj[clientFd].filesize << std::endl;
        }
    }
    else if (Readbyte == 0)
    {
        std::cout << "✅ Reading has been finished" << std::endl;
        clientobj[clientFd].send_complete = 1;
        close(clientobj[clientFd]._fd);
    }
    else
    {
        perror("read failed");
        return "Failed";
    }
    return "Done";
}

void Response::RequestResponse(int clientFd, Response &res)
{
    std::ostringstream response;
    response << "HTTP/1.0 " << res.statusCode << "\r\n"
             << "Content-Type: " << res.contentType << "\r\n"
             << "Content-Length: " << res.body.size() << "\r\n\r\n"
             << res.body;

    ssize_t sent = send(clientFd, response.str().c_str(), response.str().size(), 0);
    if (sent < 0)
        std::cerr << "❌ send failed: " << strerror(errno) << std::endl;
    else
    {
        std::cout << "✅ File response sent to FD: " << clientFd << std::endl;
        return ;
    }
    return;
}

Response Response::buildResponse(request &r, int code, const std::string &msg, const std::string &filePath, int clientFd, std::map<int, Client> &clientobj)
{
    std::cerr << "@@@@@@@Trying to open: [" << filePath << "]\n";

    Response rep;
    rep.statusCode = code;
    rep.statusMsg = msg;
    std::ifstream file(filePath.c_str(), std::ios::in | std::ios::binary);// we open file in binary read mode to support text && binary files
    if (!file)
    {
        std::cerr << "❌ Failed to open file: " << filePath << std::endl;
        rep.statusCode = 404;
        rep.statusMsg  = "Not Found";
        rep.body       = "<h1>404 Not Found</h1>";
        rep.contentType = "text/html";
        return (rep);
    }
    std::ostringstream ss; // to put file content in it ;)
    ss << file.rdbuf();
    rep.body = ss.str(); //Then put in in body and it will be in response str
    rep.contentType = "text/html"; // set a default content type;// then here we specify (it's optional btw)
    if(r.get_method() == "GET")
    {
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
             send_video(clientobj, clientFd, filePath, "200 OK");
    }
    return rep;
}
