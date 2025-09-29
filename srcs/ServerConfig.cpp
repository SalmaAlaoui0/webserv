#include "../includes/ServerConfig.hpp"
#include "../includes/ConfigParser.hpp"

InvalidErrorFile::InvalidErrorFile(const std::string &msg) :errorMsg(msg){}

InvalidErrorFile::~InvalidErrorFile() throw() {}

const char* ::InvalidErrorFile::what() const throw()
{
    return errorMsg.c_str();
}

const char * ::InvalidData::what() const throw()
{
    return ("Invalid Data");
}

bool isValidIndex(std::string index)
{
    int i = 0;
    while (index[i])
    {
        if (index[i] == ' ' || index[i] == '\t' || index[i] == '/')
            return 0;
        i++;
    }
    return 1;
}

std::string trim(std::string line)
{
    size_t start = 0;
    while (start < line.size() && std::isspace(line[start]))
        ++start;

    size_t end = line.size();
    while (end > start && std::isspace(line[end - 1]))
        --end;

    return line.substr(start, end - start);
}

bool    isAllDigits(std::string string)
{
    if (!string.size())
        return 0;
    int i = 0;
    while (string[i])
    {
        if (!std::isdigit(string[i]))
            return 0;
        i++;
    }
    return (1);
}

size_t toInt(const std::string& str)
{
    std::stringstream ss(str);
    size_t result;
    ss >> result;
    return result;
}

bool CheckValidIp(std::string ip)
{
    int i = 0;
    int Dot = 0;
	std::stringstream ss(ip);
	std::string segment;
    while (ip[i])
    {
        if (ip[i] == '.')
            Dot++;
        else if (!std::isdigit(ip[i]))
            return 0;
        i++;
    }
    if (Dot != 3)
        return 0;
    while (std::getline(ss, segment, '.'))
	{
		if (toInt(segment) > 255 || toInt(segment) < 0)
			return 0;
	}
    return 1;
}

void parseListen(std::string line, std::vector<ServerConfig> &container, int i)
{
    // check ip valid numbers 
    std::string ip;
    int port;
    std::string listen = line.substr(isKey(line, "listen") + 1);
    listen = trim(listen);
    if (listen[listen.size() - 1] != ';')
        throw ::InvalidData();
    listen = listen.substr(0, listen.size() - 1);
    size_t colonpos = listen.find(':');
    ip = listen.substr(0, colonpos);
    if (ip == "localhost")
        container[i].host = "127.0.0.1";
    else
    {
        if (!isAllDigits(listen.substr(colonpos + 1)))
            throw ::InvalidData();
        if (!CheckValidIp(ip))
            throw ::InvalidData();
        container[i].host = ip;
    }
    std::stringstream p(listen.substr(colonpos + 1));
    p >> port;
    if (port > 65535 || port <= 0)
    {
        throw ::InvalidData();
    }
    container[i].port = port;    
    // std::cout << "ur listen info, host: -" << container[i].host << "- And port is: -" << container[i].port << "-" << std::endl;
}

void parseroot(std::string line, std::vector<ServerConfig> &container, int i)
{
	std::string root;
    root = line.substr(isKey(line, "root") + 1);
    root = trim(root);
	if (root[root.size() - 1] != ';' || root[root.size() - 2] == ';')
        throw ::InvalidData();
    root = root.substr(0, root.size() - 1);
	container[i].root = root;
    // make sure to test root is a valid directory
	// std::cout << "ur root is: -" << root << "-" << std::endl;
	// exit (0);
}

void parsename(std::string line, std::vector<ServerConfig> &container, int i)
{
    std::string server_name;
    server_name = line.substr(isKey(line, "server_name") + 1);
    server_name = trim(server_name);
	if (server_name[server_name.size() - 1] != ';' || server_name[server_name.size() - 2] == ';')
        throw ::InvalidData();
    server_name = server_name.substr(0, server_name.size() - 1);
	container[i].server_name = server_name;
    // std::cout << "ur server name is: -" << container[i].server_name << "-" << std::endl;
    // exit (0);
}

void parsepages(std::string line, std::vector<ServerConfig> &container, int i) // error_page 404 403 500 /error.html;
{
    std::string error_page;
    std::string error_code;
    std::string error_path;
    error_page = line.substr(isKey(line, "error_page") + 1);
    error_page = trim(error_page);
    std::istringstream iss(error_page);
    if (!(iss >> error_code))
        throw ::InvalidData();
    if (!(iss >> error_path))
        throw ::InvalidData();
    error_path = trim(error_path);
    if (error_path[error_path.size() - 1] != ';')
        throw ::InvalidData();
    error_path = error_path.substr(0, error_path.size() - 1);
    if (!isAllDigits(error_code) || (error_path.substr(error_path.size() - 5, error_path.size()) != ".html"))
        throw ::InvalidData();
        // 400–599
    if (toInt(error_code) < 200 || toInt(error_code) > 599)
    {
        std::cerr << "Invalid error code\n";
        throw ::InvalidData();
    }
    std::ifstream file(error_path.c_str(), std::ios::in | std::ios::binary);// we open file in binary read mode to support text && binary files
    if (!file)
    {
        std::string err;
        err = "❌ Invalid page for error: ";
        err += error_code;
        throw InvalidErrorFile(err);
    }
    container[i].ErrorPages[toInt(error_code)] = error_path;
    // std::cout << "ur error page is: -" << error_page << "-" << std::endl;
    // std::cout << "     and it's error code is: -" << error_code << "-" << std::endl;
    // exit (0);
}

void parseindex(std::string line, std::vector<ServerConfig> &container, int i)
{
    std::string index;
    index = line.substr(isKey(line, "index") + 1);
    index = trim(index);
	if (index[index.size() - 1] != ';' || index[index.size() - 2] == ';')
    {
        std::cout << "Unacceptable notation `;'\n";
        throw ::InvalidData();
    }
    index = index.substr(0, index.size() - 1);
    index = trim(index);
    if (!isValidIndex(index) || (index.substr(index.size() - 5, index.size()) != ".html"))
        throw ::InvalidData();
    container[i].index = index;
    // std::cout << "ur index file is: -" << container[i].index << "-" << std::endl;
}

void parse_max_size(std::string line, std::vector<ServerConfig> &container, int i)
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
        container[i].client_max_body_size = toInt(size);
    else if (max_size[l - 1] == 'K')
        container[i].client_max_body_size = (toInt(size) * 1024);
    else if (max_size[l - 1] == 'M')
        container[i].client_max_body_size = (toInt(size) * 1024 * 1024);
    else if (max_size[l - 1] == 'G')
        container[i].client_max_body_size = (toInt(size) * 1024 * 1024 * 1024);
    // std::cout << "ur max sizeValue is: -" << container[i].client_max_body_size << "-" << std::endl;
    // exit (0);
}

void parseServerConfig(std::string line, std::vector<ServerConfig> &container, int i)
{
    if (emptylines(line) || !noClosing(line))
        return ;
    if (isKey(line, "listen"))
        parseListen(line, container, i);
	else if (isKey(line, "root"))
		parseroot(line, container, i);
    else if (isKey(line, "server_name"))
		parsename(line, container, i);
    else if (isKey(line, "error_page"))
		parsepages(line, container, i);
    else if (isKey(line, "index"))
		parseindex(line, container, i);
    else if (isKey(line, "client_max_body_size"))
		parse_max_size(line, container, i);
    else if (!isKey(line, "server"))
    {
        std::cout << line << std::endl;
        throw ::InvalidData();
    }
}
// maybe I need to handle when it ends with ;;
// if (anything except location check that the line ends by ;)
