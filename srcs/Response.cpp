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
void Response::RequestResponse(int clientFd, Response &res, std::map<int, Client> &clientobj, EpollManager &epoll)
{
    std::ostringstream response;
    ssize_t sent = 0;
    (void)epoll;
    std::cout << clientobj[clientFd].method << "<==== this is the method and has cgi is; " << clientobj[clientFd].has_cgi << "\n\n";
    std::cout << "and infos are:  the sending var is->" << clientobj[clientFd].Sending << " and pipefd->" << clientobj[clientFd].cgiMap[clientFd].pipefd << std::endl;
//    if (clientobj[clientFd].Sending)
//    {
//     std::cout << "headers are sent and in body there is: " << clientobj[clientFd].CgiBody << std::endl;
//         exit (66);
//    
    // if (clientobj[clientFd].has_cgi && clientobj[clientFd].Sending == 0 && !clientobj[clientFd].send_complete)
    if (clientobj[clientFd].has_cgi && clientobj[clientFd].Sending == 0 && clientobj[clientFd].cgiMap[clientFd].pipefd == -1)
    {
        if (clientobj[clientFd].CgiBody.find("\r\n\r\n") != std::string::npos)
        {
            size_t HeaderEnd = clientobj[clientFd].CgiBody.find("\r\n\r\n");
            clientobj[clientFd].CgiBody.substr(HeaderEnd + 1);
            // std::cout << "hahahahahah found headers\n";
            // exit(23);
        }
        std::cout << "come to here\n";
        std::ostringstream headers;
        // std::cout << "Headers are starus code is; " << res.statusCode << "content type is: " << res.contentType 
        headers << "HTTP/1.1 " << 200 << "\r\n"
            << "Content-Type: " << "text/html" << "\r\n";
        // if(clientobj[clientFd].has_cookie == 0)  //zadt cookies
        // {
        //     headers<< "Set-Cookie: session_id=" << res.sessionId << "\r\n";
        //     clientobj[clientFd].has_cookie = 1;
        // }
            headers<< "Content-Length: " << clientobj[clientFd].CgiBody.size() << "\r\n"
            << "Connection: : keep-alive\r\n\r\n";

        std::string headerStr = headers.str();
        std::cout << "here1 responseChunked is: " << clientobj[clientFd].ResponseChunked << "\n" ;// keep those u'll need them 
        send(clientFd, headerStr.c_str(), headerStr.size(), MSG_NOSIGNAL);
        clientobj[clientFd].Sending = 1;
        // std::cout.write(clientobj[clientFd].CgiBody.c_str(), clientobj[clientFd].CgiBody.size());
        // std::cout << "AGAING it will enter here just make the code\n";
        // exit (34);
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
        std::cout << "here1\n";// keep those u'll need them 
        send(clientFd, headerStr.c_str(), headerStr.size(), MSG_NOSIGNAL);
        clientobj[clientFd].Sending = 1;
    }
    else if (clientobj[clientFd].method == "GET" && clientobj[clientFd].has_cgi && clientobj[clientFd].Sending == 1
        && !clientobj[clientFd].send_complete && clientobj[clientFd].cgiMap[clientFd].pipefd == -1)
    {
        // std::cout << "ur buffer size is: " << res.body.size() << " and readbyte are: " << Readbyte << "<----\n\n";
        std::cout << "here222\n";// keep those u'll need them 
        // exit (34);
        // if (clientobj[clientFd].response.body.find("\r\n\r\n") != std::string::npos)
        // {
        //     size_t HeaderEnd = clientobj[clientFd].PostBody.find("\r\n\r\n");
        //     clientobj[clientFd].response.body.substr(HeaderEnd + 1);
        //     std::cout << "hahahahahah found headers\n";
        // }
        if (!clientobj[clientFd].CgiBody.size())
        {
            std::cout << "making the send as complete.. but before the Cgi Body is: \n" << std::endl;
            std::cout << clientobj[clientFd].CgiBody << std::endl;
            // std::cout << "Removing CGI pipe fd " << clientobj[clientFd].cgiMap[clientFd].pipefd << " from epoll\n";
            //     epoll_ctl(epoll.getEpollFd(), EPOLL_CTL_DEL, clientobj[clientFd].cgiMap[clientFd].pipefd, NULL);
            //     close(clientobj[clientFd].cgiMap[clientFd].pipefd);
            clientobj[clientFd].send_complete = 1;
        }
        // exit(23);
        ssize_t sendbytes = send(clientFd, clientobj[clientFd].CgiBody.c_str(), clientobj[clientFd].CgiBody.size(), MSG_NOSIGNAL);
        if (sendbytes != -1)
        {
            clientobj[clientFd].size_send += sendbytes;
            // std::cout << "byte sended are : " << clientobj[clientFd].size_send << "and file size is: " << clientobj[clientFd].filesize << std::endl;
        }
        if (clientobj[clientFd].CgiBody.size() && sendbytes > 0)
        {
            if (static_cast<unsigned long>(sendbytes) == clientobj[clientFd].CgiBody.size())
                clientobj[clientFd].CgiBody = "";
            else
                clientobj[clientFd].CgiBody.substr(sendbytes + 1);
        }

        // clientobj[clientFd].CgiBody.substr(sendbytes + 1);
    }
    else if (!clientobj[clientFd].has_cgi && clientobj[clientFd].method == "GET" && clientobj[clientFd].Sending == 1
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
    else if (!clientobj[clientFd].has_cgi)
    {
        response << "HTTP/1.0 " << clientobj[clientFd].response.statusCode << "\r\n"
                << "Content-Type: " << clientobj[clientFd].response.contentType << "\r\n";
                if(clientobj[clientFd].has_cookie == 0)  //zadt cookies
                {
                    response << "Set-Cookie: session_id=" << clientobj[clientFd].response.sessionId <<"\r\n";
                    clientobj[clientFd].has_cookie = 1;
                }
                response << "Content-Length: " << clientobj[clientFd].response.body.size() << "\r\n\r\n"
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
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return;
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

std::string generateId(size_t length = 16) 
{
    const char set[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    std::string result;
    result.reserve(length);

    for (size_t i = 0; i < length; i++)
        result += set[rand() % (sizeof(set) - 1)];
    return result;
}

Response Response::buildResponse(request &r, int code, const std::string &msg, const std::string &filePath, int clientFd, std::map<int, Client> &clientobj)
{
    Response rep;
    Server s;
    if(clientobj[clientFd].has_cookie == 0)  //zadt cookies
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
      //  exit(43);
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
