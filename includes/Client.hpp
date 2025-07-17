#pragma once

#include <iostream>
#include <unistd.h>
#include <ctime>

class Client
{
    private:
        int _fd;
        // std::string _requestBuffer;
        // std::string _responseBuffer;
        std::time_t lastActivity;
    public :
        Client();
        Client(int fd);
        ~Client();
        int getFd() const;
        int getLastActivity() const;
        void updateActivity();
        void receiveRequest();
        void sendResponse();
};
