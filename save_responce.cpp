#include "../includes/Server.hpp"
#include "../includes/Response.hpp"
#include "../includes/Request.hpp"
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
            std::cout << "✅ Reading has been finished" << std::endl;
            clientobj[clientFd].send_complete = 1;
            close(clientobj[clientFd]._fd);
        }
        else if (Readbyte > 0)
        {
            rep.body.assign(buffer, Readbyte);
            // std::cout << "I read ==>" << rep.body << std::endl;
            rep.Readbyte = Readbyte;
            std::cout << "and the size of it is: ==>" << rep.Readbyte << std::endl;
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

void Response::RequestResponse(int clientFd, Response &res, std::map<int, Client> &clientobj, request &r, std::vector<ServerConfig> _configs)
{
    (void)r;
    (void)_configs;
    std::ostringstream response;
    ssize_t sent = 0;

    // std::cout << clientobj[clientFd].method << "<==== this is the method and has cgi is; " << clientobj[clientFd].has_cgi << "\n\n";
    // std::cout << "and infos are:  the sending var is->" << clientobj[clientFd].Sending << " and pipefd->" << clientobj[clientFd].cgiMap[clientFd].pipefd << std::endl;
//    if (clientobj[clientFd].Sending)
//    {
//     std::cout << "headers are sent and in body there is: " << clientobj[clientFd].CgiBody << std::endl;
//         exit (66);
//    
    // if (clientobj[clientFd].has_cgi && clientobj[clientFd].Sending == 0 && !clientobj[clientFd].send_complete)
    if (clientobj[clientFd].statusCode == 302)
    {
        // clientobj[clientFd].statusCode = 0;
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

        // exit(32);
    }
    else if (clientobj[clientFd].has_cgi && clientobj[clientFd].Sending == 0 && clientobj[clientFd].Read && !clientobj[clientFd].cgi_has_problem)
    {
        if (clientobj[clientFd].CgiBody == "Content-Type: text/plain\r\n\r\nexecve failed\n")
        {
            clientobj[clientFd].statusCode = 500;
            clientobj[clientFd].statusMsg = "Internal Server Error";
        }
        else 
        {
            clientobj[clientFd].statusCode = 200;
            clientobj[clientFd].statusMsg = "OK";
        }
        if(clientobj[clientFd].has_cookie == 0)  //zadt cookies
        {
            srand(time(NULL));
            clientobj[clientFd].sessionId = generateId(16);
            clientobj[clientFd].getSession().push_back(clientobj[clientFd].sessionId);
            std::cout << "Set-Cookie: session_id=" << clientobj[clientFd].sessionId << "\n";
            std::cout << "Hello, new user! Data saved on server.\n\n";
        }
        if (clientobj[clientFd].CgiBody.find("Content-Type:") != std::string::npos)
        {
            size_t contentType = clientobj[clientFd].CgiBody.find("Content-Type:");
            std::string headers = clientobj[clientFd].CgiBody.substr(contentType + 14);
            clientobj[clientFd].CgiBody = clientobj[clientFd].CgiBody.substr(contentType + 14);
            contentType = headers.find("\n");
            if (contentType != std::string::npos)
                clientobj[clientFd].ContentType = headers.substr(0, contentType - 1);
            clientobj[clientFd].CgiBody = clientobj[clientFd].CgiBody.substr(contentType + 3);
            // std::cout << "hahahahahah found Content-Type: -" << clientobj[clientFd].ContentType << "- and the new cgi body is: -" << clientobj[clientFd].CgiBody << "--\n";
            // exit(23);
        }
        // if (!clientobj[clientFd].CgiBody.empty())
        // {
        //     std::cout << "Body is not empty()\n";
        //     exit(23);
        // }
        if (clientobj[clientFd].CgiBody.find("\r\n\r\n") != std::string::npos)
        {
            size_t HeaderEnd = clientobj[clientFd].CgiBody.find("\r\n\r\n");
            std::string headers = clientobj[clientFd].CgiBody.substr(0, HeaderEnd);
            clientobj[clientFd].CgiBody = clientobj[clientFd].CgiBody.substr(HeaderEnd + 1);
        }
        std::cout << "come to here\n";
        std::ostringstream headers;
        // headers << "HTTP/1.1 " << clientobj[clientFd].statusCode << " " << clientobj[clientFd].statusMsg << "\r\n"
        //     << "Content-Type: " << clientobj[clientFd].ContentType << "\r\n";
        headers << "HTTP/1.1 " << clientobj[clientFd].statusCode << " " << clientobj[clientFd].statusMsg << "\r\n"
            << "Content-Type: " << "video/mp4" << "\r\n";
        if(clientobj[clientFd].has_cookie == 0)  //zadt cookies
        {
            headers<< "Set-Cookie: session_id=" << clientobj[clientFd].sessionId << "\r\n";
            clientobj[clientFd].has_cookie = 1;
        }
            headers << "Connection: keep-alive\r\n\r\n"; ////<< "Content-Length: " << clientobj[clientFd].CgiBody.size() << "\r\n" <--- removed this because the response is chuncked

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
        std::cout << "here1 and filesize is: " << res.filesize << "\n";// keep those u'll need them 
        send(clientFd, headerStr.c_str(), headerStr.size(), MSG_NOSIGNAL);
        clientobj[clientFd].Sending = 1;
    }
    else if (clientobj[clientFd].method == "GET" && clientobj[clientFd].has_cgi && clientobj[clientFd].Sending == 1
        && !clientobj[clientFd].send_complete && clientobj[clientFd].Read)
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
        // if (!clientobj[clientFd].CgiBody.size())
        // {
        //     std::cout << "making the send as complete.. but before the Cgi Body is: \n" << std::endl;
        //     std::cout << clientobj[clientFd].CgiBody << std::endl;
        //     // std::cout << "Removing CGI pipe fd " << clientobj[clientFd].cgiMap[clientFd].pipefd << " from epoll\n";
        //     //     epoll_ctl(epoll.getEpollFd(), EPOLL_CTL_DEL, clientobj[clientFd].cgiMap[clientFd].pipefd, NULL);
        //     //     close(clientobj[clientFd].cgiMap[clientFd].pipefd);
        //     clientobj[clientFd].send_complete = 1;
        // }
        // exit(23);
        ssize_t sendbytes = send(clientFd, clientobj[clientFd].CgiBody.c_str(), clientobj[clientFd].bytesRead, MSG_NOSIGNAL);
        std::cout << "AND NOW the  READINNG IS: --0-- it's sending\n";
        std::cout << "----------THE NUMBER OF BYTES SENDED IS: " << sendbytes << "-----\n";
        if (sendbytes != -1)
        {
            clientobj[clientFd].Read = 0;
            clientobj[clientFd].size_send += sendbytes;
            // std::cout << "byte sended are : " << clientobj[clientFd].size_send << "and file size is: " << clientobj[clientFd].filesize << std::endl;
        }
        // if (clientobj[clientFd].CgiBody.size() && sendbytes > 0)
        // {
        //     if (static_cast<unsigned long>(sendbytes) == clientobj[clientFd].CgiBody.size())
        //         clientobj[clientFd].CgiBody = "";
        //     else
        //         clientobj[clientFd].CgiBody.substr(sendbytes + 1);
        // }

        // clientobj[clientFd].CgiBody.substr(sendbytes + 1);
    }
    else if (!clientobj[clientFd].has_cgi && clientobj[clientFd].method == "GET" && clientobj[clientFd].Sending == 1
        && !clientobj[clientFd].ResponseChunked && !clientobj[clientFd].send_complete)
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
    else if (!clientobj[clientFd].has_cgi || clientobj[clientFd].cgi_has_problem)
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
        // exit(7); if I have a cgi script
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
        std::cout << "data are==>: the file path is: " << filePath << " and the type is: " << rep.contentType << "\n ";
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
