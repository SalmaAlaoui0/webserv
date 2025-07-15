/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: wzahir <wzahir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/09 16:30:18 by wzahir            #+#    #+#             */
/*   Updated: 2025/07/15 23:39:17 by wzahir           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include "ServerConfig.hpp"
#include "EpollManager.hpp"
#include "Client.hpp"


struct ServerConfig;

class Server
{
    private:
        std::vector<ServerConfig> _configs;
        std::vector<int> listeningSockets;
        std::map<int, Client> clients;
    public:
        Server(const std::vector<ServerConfig>& configs);
        ~Server();
        
        void setupSockets();
        int creatListeningSocket(const std::string &ip, int port);
        void run();
        bool isListeningSocket(int fd) const;
        void acceptNewClient(int listenFd, EpollManager &epollManager);
        void closeClient(int fd, EpollManager &epollManager);
        
        class socketException : public std::exception 
        {
            private:
                std::string _msg;
            public:
                socketException(const std::string &msg);    
               virtual ~socketException() throw();    
                virtual const char* what() const throw();
        };
};