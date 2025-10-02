/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: wzahir <wzahir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/09 16:30:18 by wzahir            #+#    #+#             */
/*   Updated: 2025/08/31 21:24:56 by wzahir           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include<sstream>

#include <iostream>
#include <vector>
#include <list>
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
#include "Response.hpp"
#include "Request.hpp"
#include "Client.hpp"
//#include "Utils.hpp"
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>       // pour getaddrinfo, addrinfo, AI_PASSIVE
#include <arpa/inet.h>   // pour inet_pton, htons
#include <unistd.h>      // pour close()
#include <fcntl.h>       // pour fcntl, O_NONBLOCK
#include <cstring>       // pour memset
#include <string>        // pour std::string
#include <iostream> 

class request;
class Response;
struct ServerConfig;
class EpollManager;
class Client;

class Server
{
    private:
        std::vector<ServerConfig> _configs;
        std::vector<int> serverSockets;
        std::map<int, Client> clients;    
        std::vector<std::string> sessions;
        public:
        Server();
        Server(const std::vector<ServerConfig> &configs);
        ~Server();
        std::vector<ServerConfig> getConfig() const;
        std::vector<std::string> getSession() const;
        void setupSockets();
        int creatServerSocket(const std::string &ip, int port);
        void run();
        bool isServerSocket(int fd) const;
        void acceptNewClient(request & r, int listenFd, EpollManager &epollManager);
       // void handleClient(int clientFd, EpollManager &epollManager, std::vector<epoll_event> &events);
        void checkTimeout(std::map<int, Client> &clients, EpollManager &epoll);
        //std::string readRequest(int clientFd, EpollManager &epollManager);
        void handleRequest(int clientFd, request &r, std::map<int, Client> &clientob, EpollManager &epoll);
        
        void closeConnection(int fd, EpollManager &epollManager);
        class socketException : public std::exception 
        {
            private:
            std::string _msg;
            public:
            socketException(const std::string &msg);    
            virtual ~socketException() throw();    
            virtual const char* what() const throw();
        };
        void handle_get_methode(request &r, std::vector<ServerConfig> _configs, int clientFd, size_t conf_i, std::map<int, Client> &clientobj, EpollManager &epoll);
        void handle_post_methode(request & r, std::vector<ServerConfig> _configs, int clientFd, size_t conf_ir, std::map<int, Client> &clientobj,EpollManager &epollManager);
        void handle_delete_methode(request r, std::vector<ServerConfig> _configs, int clientFd, size_t conf_i, std::map<int, Client> clientobj);
        void dir_or_file(std::string &fullpath, int clientFd, ServerConfig &config, request &r, std::map<int, Client> clientobj);
        bool delete_dir_recursive(std::string &path, int clientFd, ServerConfig &config, request &r, std::map<int, Client> clientobj);
        void CheckDirOrFile(std::string requested_path, int clientFd, std::vector<ServerConfig> config, int i, int key, request &r, std::map<int, Client> &clientobj, EpollManager &epoll);
};


    