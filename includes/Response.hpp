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
//#include "Server.hpp"
//#include "Request.hpp"

class Client;
class request;
class Response
{
    public :
        Response();
        ~Response();
        int statusCode;
        std::string statusMsg;
        std::string body;
        std::string contentType;
        void RequestResponse(int clientFd, Response &response);
        static Response buildResponse(request &r, int code, const std::string &msg, const std::string &filePath, int clientFd, std::map<int, Client> &objclient);    
};
//std::string execute_cgi(const std::string &script_path, request &r, const std::string &interpreter);
std::string SendCGIResponse(int clientFd, const std::string &cgi_output, const std::string &default_status);
//std::string CheckDirOrFileCGI(std::string requested_path, int clientFd, std::vector<ServerConfig> config, int i, int key, request r);
std::string join_path(std::string root, std::string suffix);
std::string send_dir_list(int clientFd, std::string requested_path);

#endif