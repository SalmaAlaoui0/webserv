/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: wzahir <wzahir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/09 16:30:18 by wzahir            #+#    #+#             */
/*   Updated: 2025/07/10 12:52:44 by wzahir           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <vector>
#include <cstring>
#include <unistd.h>
//#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "ServerConfig.hpp"

struct ServerConfig;

class Server
{
    private:
        std::vector<ServerConfig> _configs;
        std::vector<int> listeningSockets;
    public:
        Server(const std::vector<ServerConfig>& configs);
        ~Server();
        
        void setupSockets();
        int creatListeningSocket(const std::string &ip, int port);
        void run();
        
        class socketException : public std::exception 
        {
            private:
                std::string _msg;
            public:
                socketException(const std::string &msg);    
                virtual const char* what() const throw();
        };
};