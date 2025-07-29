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
        std::string _requestBuffer;
        Client();
        Client(int fd);
        ~Client();
        int getFd() const;
        int getLastActivity() const;
        void updateActivity();
        void receiveRequest();
        void sendResponse();
};
