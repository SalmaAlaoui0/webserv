/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: wzahir <wzahir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/08 15:25:50 by wzahir            #+#    #+#             */
/*   Updated: 2025/07/14 16:18:54 by wzahir           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Server.hpp"

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
        const std::string &ip =_configs[i].host;
        int port = _configs[i].port;
       int sock = creatListeningSocket(ip, port);
       this->listeningSockets.push_back(sock);
    }
}

int Server:: creatListeningSocket(const std::string &ip, int port)
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        throw socketException("Socket creation failed");
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
    addr.sin_addr.s_addr = inet_addr(ip.c_str());
    struct in_addr addr1;
    std::cout << inet_ntoa(addr1) << std::endl; 
    if (addr.sin_addr.s_addr == INADDR_NONE)
    {
        close(sockfd);
        throw socketException("Invalid IP address");
    }
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
    return "_msg";
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

void Server::acceptNewClient(int fd, EpollManager &epollManager)
{
    struct sockaddr clientAddr;
    socklen_t clientLen = sizeof(clientAddr);
    int clientFd = accept(fd, (struct sockaddr *)&clientAddr, &clientLen);
    if (clientFd < 0) 
        throw socketException("accept failed");
    else
        std::cout << "New client connected on FD : " << clientFd << std::endl;
    epollManager.addSocket(clientFd);
    clients.push_back(Client(clientFd));
    //clients[clientFd] = Client(clientFd);
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
            int fd = fds[i];    
            if (isListeningSocket(fd))
                acceptNewClient(fd, epollManager);
            else
            {
                try
                {
                   // handleClient(fd);
                }
                catch(const std::exception& e)
                {
                    std::cerr << "Client fd " << fd << " error: " << e.what() << std::endl;
                    closeClient(fd, epollManager);
                }
            }
        }
    } 
}

void Server::closeClient(int fd, EpollManager &epollManager)
{
    epoll_ctl(epollManager.getEpollFd(), EPOLL_CTL_DEL, fd, NULL);
    close(fd);
    std::vector<Client>::iterator itt = std::find(clients.begin(), clients.end(), fd);
    if (itt != clients.end())
    {    
        clients.erase(itt);
        std::cout << "Closed client fd: " << fd << std::endl;
    }       
}
