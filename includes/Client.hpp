#pragma once

#include <iostream>
#include <unistd.h>
#include <ctime>
//#include"Server.hpp"
//#include "Request.hpp"
#include "Response.hpp"

class Client
{
    private:
        std::time_t lastActivity;
    public :
        int _fd;
        bool body_complete;
        bool start_sending;
        bool send_complete;
        bool Sending;
        bool file_opened;
        bool no_data;
        bool ErrorFound;
        // std::ifstream file;
        // int fd;
        size_t filesize;
        size_t size_send;
        size_t conf_i ;
        // size_t filesize;
        bool header_complete ;
        bool create_file;
        std::string header;
        std::string method;
        std::string path;
        std::string version;
        Response response;

        Client();
        Client(int fd);
        ~Client();
        int getFd() const;
        int getLastActivity() const;
        void updateActivity();
        void receiveRequest();
        void sendResponse();
};
