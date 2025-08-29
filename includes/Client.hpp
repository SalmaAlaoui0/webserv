#pragma once

#include <iostream>
#include <unistd.h>
#include <ctime>

class Client
{
    private:
        int _fd;
        // std::string _responseBuffer;
        std::time_t lastActivity;
        public :
        bool body_complete ;
        bool header_complete ;
        std::string _requestBuffer;
        size_t contentLength;
        std::string contentType;
        std::string header;
        std::string method;
        std::string path;
        std::string version;

        Client();
        Client(int fd);
        ~Client();
        int getFd() const;
        int getLastActivity() const;
        void updateActivity();
        void receiveRequest();
        void sendResponse();
};
