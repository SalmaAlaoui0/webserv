

#include "../includes/Request.hpp"

std::string error_set(request r)
{
    std::map<std::string , std::string>headers = r.get_header();
    if(r.get_method() != "GET" && r.get_method() != "POST" && r.get_method() != "DELETE")
        return e405;
    if(r.get_method() == "POST")
    {
        std::map<std::string , std::string>::iterator ptr= headers.find("Content-Length"); 
        if(ptr != headers.end())
        {
        std::string a = ptr->second;
        unsigned long b = std::atoi(a.c_str());
        if(b < 0 || (b != r.get_body().size()))
            return e400;  
        }
        else
            return e400;
    }
    if((r.get_method() == "GET" || r.get_method() == "DELETE")&& !r.get_body().size())
        return e400;
    if(r.get_version() != "HTTP/1.1")
        return e505;
    if(headers.find("Host") == headers.end())
        return e400;
    if(r.get_method().empty() || r.get_path().empty() || r.get_path().empty())
        return e400;
    const unsigned long max_body_size = 1024 * 1024; // 1 Mo
    if (r.get_body().size() > max_body_size)
        return e413; 
    return "";
}
static std::string trim(std::string &s)
{
    size_t start = s.find_first_not_of(" \t\r\n");
    size_t end = s.find_last_not_of(" \t\r\n");
   
    if (start == std::string::npos || end == std::string::npos)
        return "";
    return s.substr(start, end - start + 1);
}
int parseRequest(int client_fd)
{
    request r;
    char buffer[2048] = {0};
    ssize_t bytes_received = recv(client_fd, buffer, sizeof(buffer), 0);
    std::cout << "byte read ----------->"<< bytes_received << std::endl;
    std::cout << "request :"<< buffer <<std::endl;
    std::istringstream iss(buffer);
    std::string methode , path ,version;
    std::string line;
    std::getline(iss , line ,  '\r');
    iss.ignore();
   std::istringstream line_stream(line);
   line_stream >>  methode >> path >> version;
    r.set_method(methode);
    r.set_path(path);
    r.set_vergion(version);
while(std::getline(iss, line, '\r') && !line.empty())
{
    iss.ignore();
    size_t pos = line.find(":");
    if(pos != std::string::npos)
    {
    std::string key = line.substr(0,pos);
    key = trim(key);
    std::string value = line.substr(pos+1, line.size());
    value = trim(value);
    r.set_header(key,value);
    }
}

  std::string raw_request(buffer, bytes_received);
    unsigned long content_length = 0;
    std::map<std::string, std::string>::iterator it = r.get_header().find("Content-Length");
    if (it != r.get_header().end())
        content_length = std::strtoul(it->second.c_str(), NULL, 10);

    
    size_t pos = raw_request.find("\r\n\r\n");
    if (pos != std::string::npos)
    {
        pos += 4;
        std::string initial_body = raw_request.substr(pos);
        r.get_body().append(initial_body);
    }


    while (r.get_body().size() < content_length)
    {
        ssize_t bytes_received = recv(client_fd, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0)
            break;
        //buffer[bytes_received] = '\0';
        r.get_body().append(buffer, bytes_received);
        std::cout << "body size: " << r.get_body().size() << " / " << content_length << std::endl;
    }

    std::cout << "Final body size: " << r.get_body() << std::endl;
    std::cout << "Expected: " << content_length << std::endl;




// if(!error_set(r).empty())
//     return 1;
const char *response =
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/plain\r\n"
    "Content-Length: 13\r\n"
    "\r\n"
    "Hello, World!";
size_t total = 0;
size_t len = strlen(response);
while (total < len) {
    ssize_t sent = send(client_fd, response + total, len - total, 0);
    if (sent <= 0)
        break;
    total += sent;
}
close(client_fd);
return 0;
}
