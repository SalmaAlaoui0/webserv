#include "../includes/Client.hpp"

Client::Client() : _fd(-1)
{
    updateActivity();
    body_complete = false;
}

Client::Client(int fd)
    :  recived(0), litter_chunk(""),_fd(fd)
{
    updateActivity();
}

void Client::set_header(std::string key, std::string value){
    map[key] = value;}

std::map<std::string,std::string>& Client::get_header()   {
return map;
}

// void Client::set_header(std::string key, std::string value){
//     map[key] = value;}


// std::map<std::string,std::string>& Client::get_header()   {
//     return map;
//    }

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