#pragma once

#include <iostream>
#include <unistd.h>

class Client
{
    private:
        int _fd;
    public :
        Client(int fd);
        ~Client();
        int getFd() const;
};
