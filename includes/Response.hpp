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
        std::string contentType;
        std::string sessionId;
        void RequestResponse(int clientFd, Response &response, std::map<int, Client> &clientobj);
        static Response buildResponse(int code, const std::string msg, std::string filePath, int clientFd, std::map<int, Client> &objclient , std::vector<ServerConfig> &_configs);    
};

std::string mergePaths(std::string root, std::string suffix);
void send_dir_list(int clientFd, std::string requested_path);

#endif