/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: wzahir <wzahir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/08 15:25:50 by wzahir            #+#    #+#             */
/*   Updated: 2025/07/10 14:03:03 by wzahir           ###   ########.fr       */
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

Server ::socketException::socketException(const std::string &msg) :_msg(msg){}

const char* Server::socketException::what() const throw()
{
    return "_msg";
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

void Server:: setupSockets()
{
    for(size_t i = 0; i < _configs.size(); i++)
    {
    //     const std::string &ip =_configs[i].getIp();
    //     int port = _configs[i].getPort();
    //    int sock = creatListeningSocket(ip, port);
    //    this->listeningSockets.push_back(sock);
    }
}

void Server::run()
{
    // This is where i build the poll() loop.
}