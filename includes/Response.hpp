#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include<iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string>
#include <cstring>
#include <fstream>
#include <sstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <vector>
#include <map>
#include "EpollManager.hpp"
//#include "Server.hpp"
//#include "Request.hpp"
// #define SEND 8000

class Client;
class EpollManager;
class request;
class Response
{
    public :
        Response();
        ~Response();
        int statusCode;
        std::string statusMsg;
        std::string body;
        size_t filesize;
        ssize_t Readbyte;
        std::string contentType;
        std::string sessionId;
        void RequestResponse(int clientFd, Response &response, std::map<int, Client> &clientobj, EpollManager &epoll);
        static Response buildResponse(request &r, int code, const std::string &msg, const std::string &filePath, int clientFd, std::map<int, Client> &objclient);    
};
//std::string execute_cgi(const std::string &script_path, request &r, const std::string &interpreter);
std::string SendCGIResponse(int clientFd, const std::string &cgi_output, const std::string &default_status);
//std::string CheckDirOrFileCGI(std::string requested_path, int clientFd, std::vector<ServerConfig> config, int i, int key, request r);
std::string join_path(std::string root, std::string suffix);
void send_dir_list(int clientFd, std::string requested_path);

#endif