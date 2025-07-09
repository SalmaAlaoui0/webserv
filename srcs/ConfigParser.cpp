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

int isServer(std::string line)
{
	int i = 0;
	int j;
	while (line[i] == ' ' || line[i] == '\t')
		i++;
	j = i;
	while (line[i] && line[i] != ' ' && line[i] != '\t')
		i++;
	if (line.substr(j, i) == "server")
		return i;
	return 0;
}

bool checkClosing(std::string file, int i, int s) //// find {}
{
	int j = 0;
	std::ifstream ifile;
	std::string line;
	// bool brace = 0;
	s++;
	ifile.open(file.c_str());
	std::getline(ifile, line);
	while (j < i && std::getline(ifile, line))
		j++;
	while (std::getline(ifile, line))
	{
		s = 0;
		while (line[s] && line[s] != '}')
			s++;
		if (line[s] == '}')
			return (1);
	}
	// std::cout << "the i is: " << i << " and j is: " << j << std::endl;
	return 0;
}

bool ValidBraces(std::string file, int i, int s)
{
	int j = 0;
	std::ifstream ifile;
	std::string line;
	bool brace = 0;
	s++;
	ifile.open(file.c_str());
	std::getline(ifile, line);
	while (j < i)
	{ 
		std::getline(ifile, line);
		j++;
	}
	while (line[s] && (line[s] == ' ' || line[s] == '\t'))
		s++;
	if (line[s] == '{')
		brace = 1;
	if (line[s] && line[s] != '{')
		return 0;
	if (!line[s])
		std::getline(ifile, line);
	while (emptylines(line))
		std::getline(ifile, line);
	if (brace == 1)
	{
		// std::cout << "The brace bool var. is: " << brace << std::endl;
		if (!checkClosing(file, i, s))
			throw ConfigParser::InvalidBrace();
	}
	/// check either there is the closing for the brace or no braces at all
	std::cout << "server info starts in: " << line << std::endl;
	exit (7);
	return 1;
}

void parseConfig(std::string file)
{
	std::ifstream ifile;
	std::string line;
	int i = 0;
	ifile.open(file.c_str());
	if (ifile.fail())
        throw ConfigParser::InvalidFile();
	int dotPosition = file.find('.');
	if (dotPosition == -1)
		throw ConfigParser::InvalidFile();
	if (file.substr(dotPosition) != ".conf") // check for extention
		throw ConfigParser::InvalidFile();
	while (std::getline(ifile, line))
	{
        if (emptylines(line)) // skip empty lines
		{
            i++;
			continue;
		}
        // std::cout << "hello " << std::endl;
		else if (isServer(line)) // search starting line and braces
		{
            // std::cout << "Here there is a server" << std::endl;
        // std::cout << "here" << std::endl;
			if (!ValidBraces(file, i, isServer(line)))
				throw ConfigParser::InvalidFile();
			// 
			// here start extracting data from server
			
		}
		else
			throw ConfigParser::InvalidFile();
		i++;
	}

}
