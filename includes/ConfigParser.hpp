#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <vector>
#include <stack>
#include <map>

#include "LocationConfig.hpp"
#include "ServerConfig.hpp"

struct ServerConfig;


class ConfigParser
{
// private:
public:
std::vector<ServerConfig> serverdata;
	class InvalidFile : public std::exception
	{
	public:
		const char *what() const throw();
	};
	class InvalidBrace : public std::exception
	{
	public:
		const char *what() const throw();
	};
	ConfigParser(/* args */);
	~ConfigParser();
	std::vector<ServerConfig> parseConfig(std::string);
};

bool emptylines(std::string file);
int isKey(std::string line, std::string target);
bool checkClosing(std::string file);
int valid_return_path(std::vector<LocationConfig> locations, std::string Path, int index);

#endif
