#include "../includes/ConfigParser.hpp"

ConfigParser::ConfigParser(/* args */)
{
}

ConfigParser::~ConfigParser()
{
}

const char* ConfigParser::InvalidFile::what() const throw()
{
	return ("Invalid file!");
} 

const char* ConfigParser::InvalidBrace::what() const throw()
{
	return ("Invalid Braces!");
} 


bool emptylines(std::string line)
{
	int i = 0;
	while (line[i] == ' ' || line[i] == '\t')
		i++;
	if (line[i] == '\n' || line[i] == '\0' || line[i] == '#')
		return (1);
	return (0);
}

int isKey(std::string line, std::string target)
{
	std::istringstream iss(line);
	std::string word;
	int i = 0;
	int j;
	while (line[i] == ' ' || line[i] == '\t')
		i++;
	j = i;
	while (line[i] && line[i] != ' ' && line[i] != '\t')
		i++;
	if (line[i] == ' ' || line[i] == '\t')
		i--;
	iss >> word;
	if (word == target)
		return i;
	return 0;
}

bool checkClosing(std::string file)
{
	std::ifstream ifile;
	ifile.open(file.c_str());
	std::string line;
	std::stack<char> Checker;

	while (std::getline(ifile, line))
	{
		for (size_t i = 0; i < line.length(); ++i)
		{
			if (line[i] == '{')
				Checker.push('{');
			else if (line[i] == '}')
			{
				if (Checker.empty())
					throw ConfigParser::InvalidBrace();
				Checker.pop();
			}
		}
	}
	return Checker.empty();
}

bool ValidBraces(std::string file)
{
	std::ifstream ifile;
	std::string line;
	ifile.open(file.c_str());
	std::getline(ifile, line);
	if (!checkClosing(file))
		throw ConfigParser::InvalidBrace();
	return 1;
}

std::vector<ServerConfig> ConfigParser::parseConfig(std::string file)
{
	std::ifstream ifile;
	std::string line;
	int server_counter = -1;
	int lcounter = -1;
	int Server = 0;
	std::vector<ServerConfig> container;
	ifile.open(file.c_str());
	if (ifile.fail())
        throw ConfigParser::InvalidFile();
	int dotPosition = file.find('.');
	if (dotPosition == -1)
		throw ConfigParser::InvalidFile();
	if (file.substr(dotPosition) != ".conf")
		throw ConfigParser::InvalidFile();
	while (std::getline(ifile, line))
	{
		parseServerConfig(line, container, server_counter);
        if (emptylines(line))
			continue;
		else if (isKey(line, "server")) // search starting line and braces
		{
			server_counter++;
			lcounter = -1;
			container.push_back(ServerConfig());
			if (!ValidBraces(file))
				throw ConfigParser::InvalidFile();
			Server = 1;
		}
		else if (Server == 1)
		{
			Server = 0;
			while (std::getline(ifile, line) && !isKey(line, "server"))
			{
				if (emptylines(line))
				{
					continue;
				}
				else if (isKey(line, "location"))
				{
					// std::cout << "container number is: " << server_counter << std::endl;
    				container[server_counter].locations.push_back(LocationConfig());
					lcounter++;
					parseLocationConfig(line, container, server_counter, lcounter);
					while (std::getline(ifile, line) && noClosing(line))
					{
						parseLocationConfig(line, container, server_counter, lcounter);
					}
				}
				else
					parseServerConfig(line, container, server_counter);
			}
			if (isKey(line, "server"))
			{
				std::cout << "New server: "  << server_counter << std::endl;
				server_counter++;
				lcounter = -1;
				container.push_back(ServerConfig());
				if (!ValidBraces(file))
					throw ConfigParser::InvalidFile();
				Server = 1;
			}
		}
		else if (Server == 0)
			throw ConfigParser::InvalidFile();
	}
	return container;
}

// Do not forget to check if empty cofig instruc. for :
	// server_name localhost;
	// root www;
	// allowed_methods;
// And check location's info is btw {}