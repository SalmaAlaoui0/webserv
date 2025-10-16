/// awal haja nsayab object n7at fih data li9rit men and kola ciete nder while mahad kayn /r/t

// unsigned long content_length = 0;
// std::cout << "buffer ["<< buffer<< "]" << std::endl;
// std::map<std::string,std::string>&map = r.get_header();

// std::map<std::string, std::string>::iterator it= map.begin();
// if(it == map.end())
// {
//     std::cout << "ana khawiiiiiiiiiiiiiiiii\n";
//     return r;
// }
// for(it= map.begin() ; it!= map.end(); it++)
// {
//

#include "../includes/Request.hpp"

request &request::operator=(const request &other)
{
    if (this != &other)
    {
        body = other.body;
        method = other.method;
        version = other.version;
        path = other.path;
        map = other.map;
    }
    return *this;
}
request::~request() {}

request::request(request const &ref)
{
    *this = ref;
}

bool is_valid_content_length(const std::string &value)
{
    if (value.empty())
        return false;
    std::stringstream ss(value);
    long long number;
    char c;
    if (!(ss >> number))
        return false;
    if (ss >> c)
        return false;
    if (number < 0)
        return false;
    return true;
}

bool request::error_set(std::map<int, Client> &clients, int clientFd, ServerConfig &config, std::vector<ServerConfig> &_configs)
{
    std::map<std::string, std::string> headers = clients[clientFd].get_header();
    size_t pos = 0;
    pos = clients[clientFd].path.find("..");

    if (clients[clientFd].HeaderEnd > 8000)
    {
        clients[clientFd].response = Response::buildResponse(431, "Request Header Fields Too Large", config.ErrorPages[431], clientFd, clients, _configs);
        return 0;
    }
    if (pos != std::string::npos)
    {
        clients[clientFd].response = Response::buildResponse(400, "Bad Request", config.ErrorPages[400], clientFd, clients, _configs);
        return 0;
    }
    if (clients[clientFd].method != "GET" && clients[clientFd].method != "POST" && clients[clientFd].method != "DELETE")
    {
        clients[clientFd].response = Response::buildResponse(501, "Not Implemented", config.ErrorPages[501], clientFd, clients, _configs);
        return 0;
    }
    if (clients[clientFd].version != "HTTP/1.1" && clients[clientFd].version != "HTTP/1.0")
    {
        clients[clientFd].response = Response::buildResponse(505, "HTTP Version Not Supported", config.ErrorPages[505], clientFd, clients, _configs);
        return 0;
    }
    if (headers.find("Host") == headers.end())
    {
        clients[clientFd].response = Response::buildResponse(400, "Bad Request", config.ErrorPages[400], clientFd, clients, _configs);
        return 0;
    }
    if (clients[clientFd].method.empty() || clients[clientFd].path.empty())
    {
        clients[clientFd].response = Response::buildResponse(400, "Bad Request", config.ErrorPages[400], clientFd, clients, _configs);
        return 0;
    }
    if (clients[clientFd].method == "POST")
    {
        std::map<std::string, std::string>::iterator ptr = headers.find("Content-Length");
        if (ptr == headers.end() && clients[clientFd].method == "POST")
        {
            clients[clientFd].response = Response::buildResponse(411, "Length Required", config.ErrorPages[411], clientFd, clients, _configs);
            return 0;
        }
        if (ptr != headers.end())
        {
            if (!is_valid_content_length(ptr->second))
            {
                clients[clientFd].response = Response::buildResponse(411, "Length Required", config.ErrorPages[411], clientFd, clients, _configs);
                return 0;
            }
            if (std::atof(ptr->second.c_str()) > static_cast<double>(_configs[clients[clientFd].conf_i].client_max_body_size))
            {
                clients[clientFd].response = Response::buildResponse(413, "Payload Too Large", config.ErrorPages[413], clientFd, clients, _configs);
                return 0;
            }
        }
    }
    if (clients[clientFd].PostBody.size() != (size_t)std::atoi(clients[clientFd].get_header()["Content-Length"].c_str()) && clients[clientFd].method == "POST")
    {
        clients[clientFd].response = Response::buildResponse(400, "bad request", config.ErrorPages[400], clientFd, clients, _configs);
        return 0;
    }
    if (clients[clientFd].field_open)
        return 0;
    return 1;
}

std::string trim1(std::string &s)
{
    size_t start = s.find_first_not_of(" \t\r\n");
    size_t end = s.find_last_not_of(" \t\r\n");

    if (start == std::string::npos || end == std::string::npos)
        return "";
    return s.substr(start, end - start + 1);
}
request::request()
{
    reponse_status = 0;
    body = "";
    method = "";
    path = "";
    body_chnked = "";
    fullUploadpath = "";
    ContentType = "";
    ContentLength = 0;
}

void request::set_method(std::string m) { method = m; }

void request::set_path(std::string p) { path = p; }

void request::set_vergion(std::string v) { version = v; }

void request::set_header(std::string key, std::string value)
{
    map[key] = value;
}

std::string request::get_method() { return method; }

std::string request::get_version() { return version; }

std::string request::get_path() { return path; }

std::map<std::string, std::string> &request::get_header()
{
    return map;
}
void request::set_body(std::string &b) { body = b; }

std::string &request::get_body(void) { return body; }

std::vector<std::string> parseCookies(const std::string &cookieHeader)
{
    std::vector<std::string> cookies;
    std::istringstream ss(cookieHeader);
    std::string token;
    while (std::getline(ss, token, ';'))
    {
        size_t pos = token.find('=');
        if (pos != std::string::npos)
        {
            // std::string key = token.substr(0, pos);
            std::string value = token.substr(pos + 1);
            // remove spaces
            // key.erase(0, key.find_first_not_of(" "));
            cookies.push_back(value);
        }
    }
    return cookies;
}

std::string findCookies(const std::string &cookieHeader)
{
    std::vector<std::string> cookies;
    std::istringstream ss(cookieHeader);
    std::string token;
    if (std::getline(ss, token, ';'))
    {
        size_t pos = token.find('=');
        if (pos != std::string::npos)
        {
            return (token.substr(pos + 1));
        }
    }
    return NULL;
}
int file(std::map<int, Client> &clientobj, std::vector<ServerConfig> &_configs, int clientFd, request &r)
{
    struct stat statbuf;
    std::map<int, std::string> map;
    map = getMatchingRootPath(r, _configs[clientobj[clientFd].conf_i]);
    std::ostringstream filename;
    std::string fullpath = map.begin()->second;
    fullpath = mergePaths(_configs[clientobj[clientFd].conf_i].locations[map.begin()->first].root, _configs[clientobj[clientFd].conf_i].locations[map.begin()->first].upload_store);
    srand(time(NULL));
    filename << fullpath << "/" << generateId1();
    clientobj[clientFd].filename = filename.str();
    std::ofstream out(clientobj[clientFd].filename.c_str(), std::ios::binary | std::ios::trunc);
    if (!out)
    {
        std::cout << "fieled to open file path is : " << clientobj[clientFd].filename << std::endl;
        if (stat(clientobj[clientFd].filename.c_str(), &statbuf) == -1)
            clientobj[clientFd].response = Response::buildResponse(500, "Internal Server Error", _configs[clientobj[clientFd].conf_i].ErrorPages[500], clientFd, clientobj, _configs);
        else if (access(clientobj[clientFd].filename.c_str(), W_OK) == -1)
            clientobj[clientFd].response = Response::buildResponse(403, "Forbiden", _configs[clientobj[clientFd].conf_i].ErrorPages[403], clientFd, clientobj, _configs);
        else
            clientobj[clientFd].response = Response::buildResponse(500, "Internal Server Error", _configs[clientobj[clientFd].conf_i].ErrorPages[500], clientFd, clientobj, _configs);
        std::cerr << "❌ Failed to open file: " << clientobj[clientFd].filename << std::endl;
        out.close();
        remove(clientobj[clientFd].filename.c_str());
        clientobj[clientFd].body_complete = 1;
        clientobj[clientFd].field_open = 1;
        return 0;
    }
    if (remove(clientobj[clientFd].filename.c_str()) == -1)
        return 0;
    return 1;
}
request &request::parseRequest(std::map<int, Client> &clientobj, EpollManager &epollManager, request &r, int clientFd, std::vector<ServerConfig> &_configs)
{
    Server s;

    if (clientobj[clientFd].cgiMap[clientFd].pipefd != -1)
    {
        // std::cout << "should not be parsed it's a pipe event client\n";
        return r;
    }
    char buffer[640000] = {0};
    int bytes_received = recv(clientFd, buffer, sizeof(buffer), 0);
    //std::cout << "the buffer size is: " << sizeof(buffer) << std::endl;
    if (bytes_received <= 0)
    {
        if (clientobj[clientFd].PostBody.size() != (size_t)std::atoi(clientobj[clientFd].get_header()["Content-Length"].c_str()))
        {
            clientobj[clientFd].body_complete = 1;
            return r;
        }
        s.closeConnection(clientFd, epollManager);
        throw requetetException("❌ recv failed: ");
    }
    clientobj[clientFd].PostBody.append(buffer, bytes_received);
   // std::cout << "Postbody is sizzzzzzzzz: " << clientobj[clientFd].PostBody.size() << "*************<<<\n\n";
   // //  std::cout << "body complete is: " << clientobj[clientFd].body_complete << "[[[[[[]]]]]]\n\n";
    // std::cout << "in this socket file number: " << clientFd << "=> size in header: " << clientobj[clientFd].ContentLength << " and size in body is: " << clientobj[clientFd].PostBody.size() << std::endl;
    if (clientobj[clientFd].PostBody.find("\r\n\r\n") != std::string::npos && clientobj[clientFd].header_complete == 0)
    {
        clientobj[clientFd].HeaderEnd = clientobj[clientFd].PostBody.find("\r\n\r\n");
        if (clientobj[clientFd].HeaderEnd > 8000)
            clientobj[clientFd].header_complete = 1;
        std::string headers = clientobj[clientFd].PostBody.substr(0, clientobj[clientFd].HeaderEnd);
        clientobj[clientFd].PostBody = clientobj[clientFd].PostBody.substr(clientobj[clientFd].HeaderEnd + 4);
        std::istringstream iss(headers);
        std::string methode, path, version, line;
        std::getline(iss, line, '\r');
        iss.ignore();
        std::istringstream line_stream(line);
        line_stream >> methode >> path >> version;
        if (methode.empty() || path.empty() || version.empty())
        {
            clientobj[clientFd].header_complete = 1;
            r.method = "";
            clientobj[clientFd].method = "";
            return r;
        }
        clientobj[clientFd].method = methode;
        size_t pos1;
        if ((pos1 = path.find("?")) != std::string::npos)
        {
            clientobj[clientFd].path = path.substr(0, pos1);
            clientobj[clientFd].QUERY_STRING = path.substr(pos1 + 1, path.size() - pos1);
        }
        else
            clientobj[clientFd].path = trim1(path);
        clientobj[clientFd].version = version;
        // r.set_method(method);
        // r.set_path(path);
        // r.set_vergion(version);
        while (std::getline(iss, line, '\r') && !line.empty())
        {
            iss.ignore();
            size_t pos = line.find(":");
            if (pos != std::string::npos)
            {
                std::string key = line.substr(0, pos);
                key = trim1(key);
                std::string value = line.substr(pos + 1, line.size());
                value = trim1(value);
                clientobj[clientFd].set_header(key, value);
                r.set_header(key, value);
            }
        }
        std::map<std::string, std::string>::iterator iterator;
        iterator = clientobj[clientFd].get_header().begin();
        int count = 0;
        while (iterator != clientobj[clientFd].get_header().end())
        {
            if (iterator->first == "Content-Length")
            {
                count++;
                std::stringstream ss(iterator->second);
                ss >> clientobj[clientFd].ContentLength;
                if (!is_valid_content_length(iterator->second))
                    clientobj[clientFd].body_complete = 1;
                if (std::atof(iterator->second.c_str()) > static_cast<double>(_configs[clientobj[clientFd].conf_i].client_max_body_size))
                    clientobj[clientFd].body_complete = 1;
            }
            if (iterator->first == "Content-Type")
                clientobj[clientFd].ContentType = iterator->second;
            if (iterator->first == "Cookie")
            {
                clientobj[clientFd].cookies = iterator->second;
                std::string id = findCookies(clientobj[clientFd].cookies);
                // std::cout << "Session ID: " << id << std::endl;
                clientobj[clientFd].has_cookie = 1;
            }
            iterator++;
        }
        if (count == 0)
            clientobj[clientFd].body_complete = 1;
        clientobj[clientFd].header_complete = 1;
    }
    if (clientobj[clientFd].header_complete)
    {
        r.set_method(clientobj[clientFd].method);
        r.set_path(clientobj[clientFd].path);
        r.set_vergion(clientobj[clientFd].version);
        if (!file(clientobj, _configs, clientFd, r))
            return r;
        if (clientobj[clientFd].ContentLength == clientobj[clientFd].PostBody.size() || clientobj[clientFd].method == "GET")
        {
            clientobj[clientFd].body_complete = 1;
            if (clientobj[clientFd].method == "POST")
            {
                std::cout << "\nReading Post body is Done ✅\n";
                clientobj[clientFd].send_complete = 1;
            }
        }
    }

    return r;
}

request::requetetException::requetetException(const std::string &msg) : _msg(msg) {}

request::requetetException::~requetetException() throw() {}

const char *request::requetetException::what() const throw()
{
    return (this->_msg).c_str();
}
int request::get_final_port(request &r)
{
    std::map<std::string, std::string> map = r.get_header();
    std::map<std::string, std::string>::iterator it;
    for (it = map.begin(); it != map.end(); it++)
    {
        if (it->first == "Host")
        {
            int pos = it->second.find(":");
            std::string port = it->second.substr(pos + 1, it->second.size());
            int final_port = std::atoi(port.c_str());
            return final_port;
        }
    }
    return 0;
}
