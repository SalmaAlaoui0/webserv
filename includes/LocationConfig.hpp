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
	std::map<std::string, std::string> cgi_pass;  // done ✅
	std::map<int, std::string> Return;  // done ✅
	bool autoindex;  // done ✅
	std::string upload_store;
	// parse a return key

    std::vector<std::string> allowed_methods;  // done ✅
};

void parseLocationConfig(std::string line, std::vector<ServerConfig> &container, int, int);
bool noClosing(std::string line);

#endif

