

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
int parce_request(int client_fd)
{
    request r;
    char buffer[2048] = {0};
    //read(client_fd,buffer, sizeof(buffer));
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
    while(!key.empty() &&(key[0] == ' ' || key[0] == '\t'))
        key.erase(0,1);
    std::string value = line.substr(pos+1, line.size());
    while(!value.empty() &&(value[0] == ' ' || value[0] == '\t'))
        value.erase(0,1);
    r.set_header(key,value);
    }
}
//while()
iss.ignore();
std::string raw_request = buffer;
size_t pos = raw_request.find("\r\n\r\n");
if (pos != std::string::npos)
{
    pos += 4;
    std::string body = raw_request.substr(pos);
    r.set_body(body);
}

std::cout <<  "boyyyyyyyyyyyyyyyy" <<r.get_body()<< std::endl;
if(!error_set(r).empty())
    return 1;
return 0;
}
