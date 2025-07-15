/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: wzahir <wzahir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/08 15:25:50 by wzahir            #+#    #+#             */
/*   Updated: 2025/07/15 16:14:44 by wzahir           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "../includes/Server.hpp"
#include "../includes/Request.hpp"
#include "../includes/ServerConfig.hpp"
#include <sstream>
#include<map>

Server::Server(const std::vector<ServerConfig>& configs)
{
    this->_configs = configs;
    setupSockets();
}

Server::~Server()
{
    for (size_t i = 0; i < listeningSockets.size(); ++i)
        close(listeningSockets[i]);
}

void Server:: setupSockets()
{
    for(size_t i = 0; i < _configs.size(); i++)
    {
        //std::cout<<"i---->" <<i<<"         "<<_configs[i].port<<std::endl;
        if ( _configs[i].port != 0)
        {
            const std::string &ip =_configs[i].host;
            int port = _configs[i].port;
           int sock = creatListeningSocket(ip, port);
           this->listeningSockets.push_back(sock);
        }
    }
}

int Server::creatListeningSocket(const std::string &ip, int port)
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        throw socketException("Socket creation failed");
    if (fcntl(sockfd, F_SETFL, O_NONBLOCK) == -1)
    {
        close(sockfd);
        throw socketException("Failed to set non-blocking mode on socket");
    }
    int option = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)) < 0)
    {
        close(sockfd);
        throw socketException("Socket setup failed in setsockopt()");
    }
    sockaddr_in addr;
    std::memset(&addr, 0,sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    //std::cout<< "ip avant[" <<ip << "]"<<"["<<port <<"]"<<std::endl;
    if (inet_pton(AF_INET, ip.c_str(), &addr.sin_addr) <= 0) 
    {
    close(sockfd);
    throw socketException("Invalid IP address");
    }
    //std::cout<< "[" <<ip << "]"<<"["<<port <<"]"<<std::endl;

    if(bind(sockfd, (sockaddr *)&addr, sizeof(addr)) < 0)
    {
        close(sockfd);
        throw socketException("bind() failed"); 
    }
    if(listen(sockfd, SOMAXCONN) < 0)
    {
        close(sockfd);
        throw socketException("listen() failed");    
    }
    std::cout << "Listening on " << ip << ":" << port << std::endl;
    return sockfd; 
}

Server ::socketException::socketException(const std::string &msg) :_msg(msg){}

Server ::socketException::~socketException() throw() {}


const char* Server::socketException::what() const throw()
{
    return (this->_msg).c_str();
}

bool Server::isListeningSocket(int fd) const 
{
    for (size_t i = 0; i < listeningSockets.size(); ++i) 
    {
        if (listeningSockets[i] == fd)
            return true;
    }
    return false;
}

void Server::acceptNewClient(int listenFd, EpollManager &epollManager)
{
    // while (true)
    // {
        struct sockaddr clientAddr;
        socklen_t clientLen = sizeof(clientAddr);

        int clientFd = accept(listenFd, (struct sockaddr *)&clientAddr, &clientLen);
        if (clientFd < 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)//break;
            throw socketException("accept failed");
        }
      
        if (fcntl(clientFd, F_SETFL, O_NONBLOCK) == -1)
        {
            close(clientFd);
            throw socketException("fcntl() failed on client FD");
        }
        std::cout << "New client connected on FD : " << clientFd << std::endl;
        epollManager.addSocket(clientFd);
        clients.push_back(Client(clientFd));
    //}
    //clients[clientFd] = Client(clientFd);
}

std::string readRequest(int clientFd)
{
    char buffer[1024]="";
    ssize_t bytesRecv=recv(clientFd, buffer, sizeof(buffer) - 1, 0 );
    if (bytesRecv == -1)
    {
        if (errno != EAGAIN && errno != EWOULDBLOCK)
        {
            std::cerr << "recv failed: " << strerror(errno) << std::endl;
            //close(clientFd);
        }
    }
    else if (bytesRecv == 0)
    {
        std::cout << "Client disconnected" << std::endl;
    }
    else   
        buffer[bytesRecv] = '\0';
    std::cout<< "Received: " << buffer<<std::endl;
    return (std::string)buffer;
}

void sendResponse( int clientFd)
{
    std::cout << "------->hi\n";
    const char* response =
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/plain\r\n"
    "Content-Length: 13\r\n"
    "\r\n"
    "Hello, World!";
    ssize_t sent =send(clientFd, response, strlen(response), 0); 
     if (sent < 0)
        std::cerr << "send failed: " << strerror(errno) << std::endl;
    else
        std::cout << "[sendResponse] Response sent to FD: " << clientFd << std::endl;
}
void handleClient(int clientFd)
{
    while (true)
    {
        std::string request = readRequest(clientFd);
        if (request.empty())
        {
            std::cout << "[handleClient] Empty request or client closed" << std::endl;
            close(clientFd);
            return;
        }
        if (parseRequest(request) == 0)
            sendResponse(clientFd);
        close(clientFd);   
    }
}

void Server::run()
{
    EpollManager epollManager;
    for (size_t i = 0; i < listeningSockets.size(); i++)
    {
        epollManager.addSocket(listeningSockets[i]);
    }
    while (true)
    {
        std::vector<int> fds = epollManager.waitEvents(1000);
        for (size_t i = 0; i < fds.size(); i++)
        {
            // std::cout << "[EVENT] FD = " << fds[i]<< std::endl;
            int fd = fds[i];    
            if (isListeningSocket(fd))
                acceptNewClient(fd, epollManager);
            else
            {
                // try
                // {
                //     std::cout<<"hiiii i'am in i==="<<i<<fds[i]<<std::endl;
                     handleClient(fd);
                // }
                // catch(const std::exception& e)
                // {
                //     std::cerr << "Client fd " << fd << " error: " << e.what() << std::endl;
                //     closeClient(fd, epollManager);
                // }
            }
        }
    } 
}

void Server::closeClient(int fd, EpollManager &epollManager)
{
    epoll_ctl(epollManager.getEpollFd(), EPOLL_CTL_DEL, fd, NULL);
    close(fd);
     std::cout << "[closeClient] Closed FD: " << fd << std::endl;
    // std::vector<Client>::iterator itt = std::find(clients.begin(), clients.end(), fd);
    // if (itt != clients.end())
    // {    
    //     clients.erase(itt);
    //     std::cout << "Closed client fd: " << fd << std::endl;
    // }       
}
