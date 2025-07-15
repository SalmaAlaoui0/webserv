#pragma once

#include <iostream>
#include <unistd.h>

class Client
{
    private:
        int _fd;
        std::string _requestBuffer;
        std::string _responseBuffer;
    public :
        Client(int fd);
        ~Client();
        int getFd() const;
        void receiveRequest();
        void sendResponse();
};
