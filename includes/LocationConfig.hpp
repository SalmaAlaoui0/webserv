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
	std::string path;  // done ✅
	std::string root;  // done ✅
	std::string index;  // done ✅
	long	client_max_body_size;  // done ✅
	std::string cgi_pass;  // done ✅
	bool autoindex;  // done ✅

    std::vector<std::string> allowed_methods;  // done ✅
};

void parseLocationConfig(std::string line, std::vector<ServerConfig> &container, int, int);
bool noClosing(std::string line);

#endif

