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
	while (line[i] == ' ' || line[i] == '\t')
		i++;
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

int valid_return_path(std::vector<LocationConfig> locations, std::string Path, int index)
{
	size_t i = 0;
	while (i < locations.size())
	{

		std::string locpath = trim(locations[i].path);
		// std::cout << "Location is: " << i << " and it's path is: -" << locpath << "-" << std::endl;
		if (locpath == Path)
		{
			if (index == 2)
				return 1;
			else 
				return i;
		}
		i++;
	}
	return 0;
}

std::string return_found(std::vector<LocationConfig> locations, std::string Path)
{
	size_t i = 0;
	while (i < locations.size())
	{
		std::string locpath = trim(locations[i].path);
		// std::cout << "Location is: " << i << " and it's path is: -" << locpath << "-" << std::endl;
		if (locpath == Path)
		{
			if (locations[i].Return.empty())
			{
				return "";
			}
			else 
			{
				std::string value = locations[i].Return.begin()->second;
				return value;
			}
		}
		i++;
	}
	return "";
}

void Check_return(std::vector<ServerConfig> &container)
{
	size_t server = 0, loc;
	std::vector<std::string> returnPath;
	while (server < container.size())
	{
		if (container[server].host.empty())
		{
			std::cerr << "`listen ip:port;' is mondatory in server" << std::endl;
			throw ::InvalidData();
		}
		if (container[server].server_name.empty())
		{
			std::cerr << "`server_name server;' is mondatory in server" << std::endl;
			throw ::InvalidData();
		}
		if (container[server].root.empty())
		{
			std::cerr << "`a default root;' is mondatory in server" << std::endl;
			throw ::InvalidData();
		}
		loc = 0;
		while (loc < container[server].locations.size())
		{
			returnPath.clear();
			if (!container[server].locations[loc].Return.empty())
			{
				std::string value = container[server].locations[loc].Return.begin()->second;
				// std::cout << "entered condition in:--" << loc << "--location with value: " << value << std::endl;
				while (!value.empty())
				{
					if (!valid_return_path(container[server].locations, value, 2))
					{
						std::cout << "Invalid `return' value Detected" << std::endl;
						throw ::InvalidData();
					}
					value = return_found(container[server].locations, value);
					if (std::find(returnPath.begin(), returnPath.end(), value) == returnPath.end() && !value.empty())
					{
						returnPath.push_back(value);
						container[server].locations[loc].Return.begin()->second = value;
					}
					else if (!value.empty())
					{
						std::cout << "Invalid `return' Implementation Detected" << std::endl;
						throw ::InvalidData();
					}
				}
				// std::cout << "And Location: " << loc << "new return path value will be: " << container[server].locations[loc].Return.begin()->second << std::endl;
			}
			loc++;
		}
		server++;
	}
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
	if (container.empty())
	{
		std::cerr << "Error: at least one 'server' block is required in the config file" << std::endl;
		throw ConfigParser::InvalidFile();
	}
	Check_return(container);
	return container;
}

// Do not forget to check if empty cofig instruc. for :
	// server_name localhost;
	// root www;
	// allowed_methods;
// And check location's info is btw {}