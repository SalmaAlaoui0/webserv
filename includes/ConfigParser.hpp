#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <vector>
#include <stack>
#include <map>

struct	ServerConfig
{
	int port;
	std::string root;
	std::map<int, std::string> ErrorPages;
};


class ConfigParser
{
private:
	std::vector<ServerConfig> serverdata;
public:
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
};

void parseConfig(std::string);
bool emptylines(std::string file);
int isServer(std::string line);
bool ValidBraces(std::string file, int, int server);
bool checkClosing(std::string file, int i, int s);

#endif
