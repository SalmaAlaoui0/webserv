#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include<iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <iostream>
#include "Request.hpp"


class reponse{
    public : 
    int reponse_status;
    std::string response_body;
    reponse();
    ~reponse();

};

std::string RequestResponse(int clientFd, std::string filePath, std::string resCode);
std::string execute_cgi(const std::string &script_path, request &r, const std::string &interpreter);
std::string SendCGIResponse(int clientFd, const std::string &cgi_output, const std::string &default_status);
std::string CheckDirOrFileCGI(std::string requested_path, int clientFd, std::vector<ServerConfig> config, int i, int key, request r);
std::string join_path(std::string root, std::string suffix);
std::string send_dir_list(int clientFd, std::string requested_path);

#endif