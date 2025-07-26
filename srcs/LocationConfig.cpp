#include "../includes/ServerConfig.hpp"
#include "../includes/ConfigParser.hpp"
#include "../includes/LocationConfig.hpp"

int containsChar(std::string str, char target) {
	int result;
	std::vector<std::string> chunks;
	std::string word;
	result = str.find(target) != std::string::npos;
	if (result)
	{
		int j = str.find(target);
		j++;
		while (str[j])
		{
			if (str[j] != ' ' || str[j] != '\t')
				return 2;
		}
	}
	str = str.substr(0, str.find(target));
	std::istringstream iss(str);
	while (iss >> word)
	{
		chunks.push_back(word);
	}
	if (chunks.size() > 2)
		return 2;
	if (chunks.size() == 2 && (chunks[0] != "^~" && chunks[0] != "~"))
		return 2;
    if (chunks[0] == "~")
    {
        if (chunks[1][0] != '\\' || chunks[1][1] != '.')
            return 2;
    }
    return result;
}

std::string checkValidLocation(std::string line)
{
    std::string value;
    std::string path;
    value = line.substr(isKey(line, "location") + 1);
    value = trim(value);
    if (containsChar(value, '{'))
    {
		if (containsChar(value, '{') == 2)
			throw ::InvalidData();
        path = value.substr(0, value.find('{'));
    }
    else
		path = value;
    // std::cout << "the path extracted is: " << path << std::endl;
    return (path);
}

bool hasExtension(const std::string& filename)
{
    size_t dotPos = filename.rfind('.');
    return dotPos != std::string::npos && dotPos != 0 && dotPos != filename.size() - 1;
}

void parseLoc_index(std::string line, std::vector<ServerConfig> &container, int i, int j)
{
    std::string index;
    index = line.substr(isKey(line, "index") + 1);
    index = trim(index);
	if (index[index.size() - 1] != ';')
        throw ::InvalidData();
    if (!isValidIndex(index) || !hasExtension(index))
        throw ::InvalidData();
    index = index.substr(0, index.size() - 1);
    container[i].locations[j].index = index;
    // std::cout << "ur location index file is: -" << container[i].locations[j].index << "-" << std::endl;
}

void parseLoc_root(std::string line, std::vector<ServerConfig> &container, int i, int j)
{
	std::string root;
    root = line.substr(isKey(line, "root") + 1);
    root = trim(root);
	// if (root[root.creatListeningSocket() - 1] != ';' || root[root.size() - 2] == ';')
    //     throw ::InvalidData();
    root = root.substr(0, root.size() - 1);
    container[i].locations[j].root = root;
	// std::cout << "ur location root is: -" << container[i].locations[j].root << "-" << std::endl;
    // exit (0);
}

void parseAutoIndex(std::string line, std::vector<ServerConfig> &container, int i, int j)
{
    std::string value;
    value = line.substr(isKey(line, "autoindex") + 1);
    value = trim(value);
    if (value[value.size() - 1] != ';' || value[value.size() - 2] == ';')
        throw ::InvalidData();
    value = value.substr(0, value.size() - 1);
    if (value == "off")
        container[i].locations[j].autoindex = 0;
    else if (value == "on")
        container[i].locations[j].autoindex = 1;
    else
        throw ::InvalidData();
    // std::cout << "Your auto index value is: -" << container[i].locations[j].autoindex << "- " << std::endl;
    // exit (0);
}

std::string MethodeSpliter(const std::string line) {
	std::istringstream iss(line);
	std::string word;
	iss >> word;
	return word;
}

int checkValidMethod(std::string method)
{
    int i = 0;
    while (method[i] == ' ' || method[i] == '\t')
        i++;
    if (!method[i])
        return 2;
    if (method == "GET" || method == "POST" || method == "DELETE")
        return 1;
    return 0;
}

void parseMethods(std::string line, std::vector<std::string> &container)
{
    std::string remainLine = line.substr(isKey(line, "allowed_methods") + 1);
    // int i = 0;
	if (remainLine[remainLine.size() - 1] != ';' || remainLine[remainLine.size() - 2] == ';')
        throw ::InvalidData();
        
    remainLine = remainLine.substr(0, remainLine.size() - 1);
    if (checkValidMethod(remainLine) == 2)
    {
        throw ::InvalidData();
    }
	while (!remainLine.empty())
	{
        std::string method = MethodeSpliter(remainLine);
		if (!checkValidMethod(method))
            throw ::InvalidData();
		if (checkValidMethod(method) == 2)
            return ;
		if (std::find(container.begin(), container.end(), method) != container.end())
            throw ::InvalidData();
		container.push_back(method);
        if (isKey(remainLine, method) < static_cast<int>(remainLine.size()))
		    remainLine = remainLine.substr(isKey(remainLine, method) + 1);
        else
            remainLine = "";
	}
}

void parseLoc_max_size(std::string line, long &client_max_body_size)
{
    std::string max_size;
    std::string size;
    max_size = line.substr(isKey(line, "client_max_body_size") + 1);
    max_size = trim(max_size);
    if (max_size.size() < 2)
        throw ::InvalidData();
    int l = max_size.size() - 1;
	if (max_size[l] != ';' ||
        (max_size[l - 1] != 'K' && max_size[l - 1] != 'M' &&
            max_size[l - 1] != 'G' && !std::isdigit(max_size[l - 1])))
        throw ::InvalidData();
    if (std::isdigit(max_size[l - 1]))
        size = max_size.substr(0, max_size.size() - 1);
    else
        size = max_size.substr(0, max_size.size() - 2);
    if (!isAllDigits(size))
        throw ::InvalidData();
    if (std::isdigit(max_size[l - 1]))
        client_max_body_size = toInt(size);
    else if (max_size[l - 1] == 'K')
        client_max_body_size = (toInt(size) * 1024);
    else if (max_size[l - 1] == 'M')
        client_max_body_size = (toInt(size) * 1024 * 1024);
    else if (max_size[l - 1] == 'G')
        client_max_body_size = (toInt(size) * 1024 * 1024 * 1024);
    // std::cout << "ur Location max file sizeValue is: -" << client_max_body_size << "-" << std::endl;
}

void parseLoc_cgi(std::string line, std::vector<ServerConfig> &container, int i, int j)
{
	std::string cgi;
    cgi = line.substr(isKey(line, "cgi_pass") + 1);
    cgi = trim(cgi);
	if (cgi[cgi.size() - 1] != ';' || cgi[cgi.size() - 2] == ';')
        throw ::InvalidData();
    cgi = cgi.substr(0, cgi.size() - 1);
    container[i].locations[j].cgi_pass = cgi;
    // std::cout << "ur cgi pass is: -" << container[i].locations[j].cgi_pass << "-" << std::endl;
    // exit (0);
}

void parseLocationConfig(std::string line, std::vector<ServerConfig> &container, int i, int j)
{
    std::vector<std::string> methods;
    if (isKey(line, "location"))
        container[i].locations[j].path = checkValidLocation(line);
    else if(isKey(line, "index"))
		parseLoc_index(line, container, i, j);
    else if(isKey(line, "root"))
		parseLoc_root(line, container, i, j);
    else if(isKey(line, "autoindex"))
		parseAutoIndex(line, container, i, j);
    else if(isKey(line, "client_max_body_size"))
        parseLoc_max_size(line, container[i].locations[j].client_max_body_size);
    else if(isKey(line, "cgi_pass"))
		parseLoc_cgi(line, container, i, j);
    else if(isKey(line, "allowed_methods"))
    {
        parseMethods(line, methods);
        container[i].locations[j].allowed_methods = methods;
    }
}

bool noClosing(std::string line)
{
	int brace = 0;
    size_t i = 0;
	while (i < line.length())
    {
		char c = line[i];
		if (c == '}')
			brace++;
		else if (brace && !isspace(c))
			throw ::InvalidData();
        i++;
	}
	if (brace == 1)
		return 0;
	else if (brace > 1)
		throw InvalidData();

	return 1;
}
