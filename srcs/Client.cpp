#include "../includes/Client.hpp"

Client::Client(int fd) : _fd(fd){}

Client::~Client()
{
    close (_fd);
}

int Client::getFd() const
{
    return(_fd);
}
