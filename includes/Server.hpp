/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: salaoui <salaoui@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/09 16:30:18 by wzahir            #+#    #+#             */
/*   Updated: 2025/08/26 11:16:15 by salaoui          ###   ########.fr       */
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
#include "Request.hpp"
#include "Client.hpp"
#include "Utils.hpp"

class request;
struct ServerConfig;
class EpollManager;
class Server
{
    private:
        std::vector<ServerConfig> _configs;
        std::vector<int> serverSockets;
        std::map<int, Client> clients;
    public:
        Server();
        Server(const std::vector<ServerConfig>& configs);
       ~Server();
        
        void setupSockets();
        int creatServerSocket(const std::string &ip, int port);
        void run();
        bool isServerSocket(int fd) const;
        void acceptNewClient(int listenFd, EpollManager &epollManager);
        void handleClient(int clientFd, EpollManager &epollManager);
        void checkTimeout(std::map<int, Client> &clients, EpollManager &epoll);
        std::string readRequest(int clientFd, EpollManager &epollManager);
        void sendResponse(int clientFd, request &r);
        
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
void send_response(int clientFd, int status_code, const std::string &status_text, const std::string &body);    
void handle_get_methode(request r, std::vector<ServerConfig> _configs, int clientFd, size_t conf_i);
void handle_post_methode(request & r, std::vector<ServerConfig> _configs, int clientFd, size_t conf_i);
void handle_delete_methode(request r, std::vector<ServerConfig> _configs, int clientFd, size_t conf_i);

