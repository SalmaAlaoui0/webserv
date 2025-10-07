#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP

#include "ConfigParser.hpp"
#include "LocationConfig.hpp"
#include <cstdlib>
#include <sstream>
#include <cctype>

struct	ServerConfig
{
	std::string host;
	int port;
	std::string root;
	std::string server_name;
	std::string index;
	size_t client_max_body_size;

	std::map<int, std::string> ErrorPages;
	std::vector<LocationConfig> locations;
};

class InvalidErrorFile : public std::exception
{
private:
	std::string errorMsg;
public:
	InvalidErrorFile(const std::string &msg);    
    virtual ~InvalidErrorFile() throw();
	virtual const char* what() const throw();
};

class InvalidData : public std::exception
{
public:
	const char *what() const throw();
};

void parseServerConfig(std::string line, std::vector<ServerConfig> &container, int);
bool isValidIndex(std::string index);
bool    isAllDigits(std::string string);
std::string trim(std::string line);
size_t toInt(const std::string& str);


#endif