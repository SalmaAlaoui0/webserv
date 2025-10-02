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
int Client::get_final_port()
{
    std::map<std::string, std::string> map = this->map;
	std::map<std::string, std::string>::iterator it;
	for(it = map.begin() ; it != map.end(); it++)
	{
		if(it->first == "Host")
		{
			int pos = it->second.find(":");
			std::string port = it->second.substr(pos+1, it->second.size());
			int final_port = std::atoi(port.c_str());
            return final_port;
		}
	}
    return 0;
}

void Client::set_header(std::string key, std::string value){
    map[key] = value;}

std::map<std::string,std::string>& Client::get_header()   {
return map;
}

 std::vector<std::string> Client::getSession() const  {
return sessions;
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