#include "../includes/Server.hpp"
#include "../includes/Response.hpp"
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

size_t get_fileSize(int fd)
{
    struct stat file;
    if (fstat(fd, &file) == -1 )
    {
        return -1;
    }
    return file.st_size;
    
}

// void sigpipe_handler(int signum) {
//     if (signum)
//     {}
// }

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

std::string RequestResponse(std::map<int, Client> &clientobj, int clientFd, std::string filePath, std::string resCode)
{
    std::ifstream file(filePath.c_str(), std::ios::in | std::ios::binary);// we open file in binary read mode to support text && binary files
    if (!file)
    {
        std::cerr << "❌ Failed to open file: " << filePath << std::endl;
        return ("❌ Failed to open file: ");
    }

    std::ostringstream ss; // to put file content in it ;)
    ss << file.rdbuf();
    std::string body = ss.str(); //Then put in in body and it will be in response str

    std::string contentType = "text/plain"; // set a default content type;// then here we specify (it's optional btw)
    // std::cout << "the last get path is: " << filePath << std::endl;
    //hereeeee
    if (filePath.find(".html") != std::string::npos)
        contentType = "text/html";
    else if (filePath.find(".css") != std::string::npos)
        contentType = "text/css";
    else if (filePath.find(".js") != std::string::npos)
        contentType = "application/javascript";
    else if (filePath.find(".jpg") != std::string::npos || filePath.find(".jpeg") != std::string::npos)
        contentType = "image/jpeg";
    else if (filePath.find(".png") != std::string::npos)
        contentType = "image/png";
    else if (filePath.find(".mp4") != std::string::npos)
    {
        // std::cout << "here \n\n";
        return send_video(clientobj, clientFd, filePath, "200 OK");
    }

    std::ostringstream response;// last but not least ofc building proper HTTP response!
    response << "HTTP/1.1 " << resCode << "\r\n"
             << "Content-Type: " << contentType << "\r\n"
             << "Content-Length: " << body.size() << "\r\n\r\n"
             << body;

    ssize_t sent = send(clientFd, response.str().c_str(), response.str().size(), 0);// Sending it to our lovely client
    if (sent < 0)// send failed!
        std::cerr << "❌ send failed: " << strerror(errno) << std::endl;
    else// Indicating success in our console
    {
        std::cout << "✅ File response sent to FD: " << clientFd << std::endl;
        return "Done ✅";
    }
    return ("SomeThing went wrong");
}
