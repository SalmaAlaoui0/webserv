#ifndef LOCATIONCONFIG_HPP
#define LOCATIONCONFIG_HPP

#include "ConfigParser.hpp"
#include <cstdlib>
#include <sstream>
#include <cctype>
#include <string>
#include <algorithm>

struct ServerConfig;

struct LocationConfig
{
	std::string path;
	std::string root;
	std::string index;
	long	client_max_body_size;
	std::map<std::string, std::string> cgi_pass;
	std::map<int, std::string> Return;
	bool autoindex;
	std::string upload_store;

    std::vector<std::string> allowed_methods;
};

void parseLocationConfig(std::string line, std::vector<ServerConfig> &container, int, int);
bool noClosing(std::string line);

#endif

