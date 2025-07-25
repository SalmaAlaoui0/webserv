

#include "../includes/Request.hpp"
request& request::operator=(const request& other)
{
     if (this != &other) {
            body = other.body;
            method = other.method;
            version = other.version;
            path = other.path;
            map = other.map;
        }
return *this;
}
request::request(request const &ref)
{
    *this = ref;
}

std::string error_set(request &r)
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
	
    // if((r.get_method() == "GET" || r.get_method() == "DELETE") && !r.get_body().size())
	// {
		// 	std::cout << "💡💡💡💡💡💡💡💡💡💡here is the error\n bodysize is: " << r.get_body().size() << std::endl;
		// 	return e400;
		// }
	if(r.get_version() != "HTTP/1.1")
		return e505;
	if(headers.find("Host") == headers.end())	
        return e400;
    if(r.get_method().empty() || r.get_path().empty())
	    return e400;
    const unsigned long max_body_size = 1024 * 1024; // 1 Mo
    if (r.get_body().size() > max_body_size)
		return e413;
    return "";
}

static std::string trim1(std::string &s)
{
    size_t start = s.find_first_not_of(" \t\r\n");
    size_t end = s.find_last_not_of(" \t\r\n");
   
    if (start == std::string::npos || end == std::string::npos)
        return "";
    return s.substr(start, end - start + 1);
}
 request::request()
 {

 }
void request::set_method(std::string m) {method= m;}

void request::set_path(std::string p){path = p;}

void request::set_vergion(std::string v) {version= v;}

void request::set_header(std::string key, std::string value){
    map[key] = value;}

std::string request::get_method(){return method;}

std::string request::get_version(){return version;}

std::string request::get_path(){return path;}

std::map<std::string,std::string>& request::get_header()   {
    return map;
   }
void request::set_body(std::string& b){body = b;}

std::string& request::get_body(void){return body ;}

request& request::parseRequest(int client_fd , EpollManager &epollManager, request &r)
{
    Server s;
    char buffer[2048] = {0};
    ssize_t bytes_received = recv(client_fd, buffer, sizeof(buffer), 0);
    if ( bytes_received == -1)
    {
        if (errno != EAGAIN && errno != EWOULDBLOCK)
        {
            std::cerr << "❌ recv failed: " << strerror(errno) << std::endl;
             s.closeClient(client_fd, epollManager);
            throw requestetException();
        }
    }
    else if ( bytes_received == 0)
    {
        std::cerr << "❌ Client disconnected " << client_fd << std::endl;
         throw requestetException();
    }
    else if(buffer[bytes_received] == '\0' && bytes_received == 1 )
    {
        std::cout << "Empty request or client closed" << std::endl;
         throw requestetException();
    }
    //std::cout << "byte read ----------->"<< bytes_received << std::endl;
    // std::cout << "request :"<< buffer <<std::endl;
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
    key = trim1(key);
    std::string value = line.substr(pos+1, line.size());
    value = trim1(value);
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
    // std::cout << "Expected: " << content_length << std::endl;
return r;
}

