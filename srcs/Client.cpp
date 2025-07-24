#include "../includes/Client.hpp"

Client::Client() : _fd(-1)
{
    updateActivity();
}

Client::Client(int fd) : _fd(fd)
{
    updateActivity();
}

Client::~Client()
{
    //close (_fd);
}

int Client::getFd() const
{
    return(_fd);
}

int Client::getLastActivity() const
{
    return(lastActivity);
}

void Client::updateActivity()
{
    lastActivity = std::time(NULL);
}