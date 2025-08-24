///awal haja nsayab object n7at fih data li9rit men and kola ciete nder while mahad kayn /r/t

   //unsigned long content_length = 0;
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

void request::error_set(request &r)
{
    std::map<std::string , std::string>headers = r.get_header();
    if(r.get_method() != "GET" && r.get_method() != "POST" && r.get_method() != "DELETE")
    {
        std::cout << "Method is: " << r.get_method() << std::endl; /// telnet 127.0.0.1 8080 there is a problem here the get method does not return the method but the path if we're using telnet as a client try it!
        throw requetetException("405 Method Not Allowed");
    }
    if(r.get_method() == "POST")
    {
        std::map<std::string , std::string>::iterator ptr = headers.find("Content-Length"); 
        if(ptr != headers.end())
        {
			std::string a = ptr->second;
			unsigned long b = std::atoi(a.c_str());
			if(b < 0 || (b != r.get_body().size()))
            {
                throw requetetException("400 Bad Request");
            }
        }
        else
            throw requetetException("400 Bad Request");
    }
	if(r.get_version() != "HTTP/1.1")
		 throw requetetException("505 HTTP Version Not Supported");
	if(headers.find("Host") == headers.end())	
       throw requetetException("400 Bad Request");
    if(r.get_method().empty() || r.get_path().empty())
	    throw requetetException("400 Bad Request");
    const unsigned long max_body_size = 1024 * 1024; // 1 Mo
    if (r.get_body().size() > max_body_size)
		 throw requetetException("Payload Too Large");
    return ;
}

static std::string trim1(std::string &s)
{
    size_t start = s.find_first_not_of(" \t\r\n");
    size_t end = s.find_last_not_of(" \t\r\n");
   
    if (start == std::string::npos || end == std::string::npos)
        return "";
    return s.substr(start, end - start + 1);
}
 request::request() {}

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

request& request::parseRequest(std::map<int, Client>& clientobj , EpollManager &epollManager, request &r)
{
    Server s;
    char buffer[2048] = {0};
    std::map<int,Client>::iterator it = clientobj.begin();
    ssize_t bytes_received = recv(it->first, buffer, sizeof(buffer), 0);
    if ( bytes_received == -1)
    {
        if (errno != EAGAIN && errno != EWOULDBLOCK)
        {
            s.closeClient(it->first, epollManager);
            throw requetetException("❌ recv failed: ");
        }
    }
    else if ( bytes_received == 0)
        throw requetetException("❌ Client disconnected ");
    else if(buffer[bytes_received] == '\0' && bytes_received == 1 )
        throw requetetException("Empty request or client closed");
    // std::cout << "request :"<< buffer <<std::endl;
    std::istringstream iss(buffer);
    std::string methode , path ,version, line;
    std::getline(iss , line ,  '\r');
    iss.ignore();
    std::istringstream line_stream(line);
    line_stream >>  methode >> path >> version;
    r.set_method(methode);
    r.set_path(path);
    r.set_vergion(version);
    // std::cout << "ur method and path and version are: \n" << "method: " << r.get_method() << std::endl
    // << "path: " << r.get_path() << std::endl << "version: " << r.get_version() << std::endl;
    while(std::getline(iss, line, '\r') && !line.empty())
    {
        iss.ignore();
        size_t pos = line.find(":");
        if(pos != std::string::npos)
        {
        std::string key = line.substr(0,pos);
        key = trim1(key);
        //std::cout << key<<std::endl;
        std::string value = line.substr(pos+1, line.size());
        value = trim1(value);
    // std::cout << value<<std::endl;
        r.set_header(key,value);
        }
    }
    std::string raw_request(buffer);//, bytes_received);
    size_t pos = raw_request.find("\r\n\r\n");
    if (pos != std::string::npos)
    {
        pos += 4;
        std::string initial_body = raw_request.substr(pos);
        r.get_body().append(initial_body);
    }
  
    if (r.get_header()["Transfer-Encoding"] == "chunked")
    {
        std::string body = r.get_body();
        char *buffer1;
        int b = 0;
        size_t pos2 = 0;
        size_t pos1 = body.find("\r\n",pos2);

        while( pos1 != std::string::npos)
        {
        std::string a = body.substr(pos2,pos1);
        b = std::strtol(a.c_str(),NULL,16);
        if(b <= 0)
        {
            it->second.body_complete = true;
            break;
        }
        buffer1 = new char[1024];
      //std::cout << "a ==>" << a<< std::endl;
      //std::cout << "b ==>" << b<< std::endl;
     int i =0;
      pos1 += 2;
      std::string c = body.substr(pos1,b);

    pos2 = b +pos1 + 2;
     
      while(i < b)
      {
          buffer1[i] = c[i];
          i++;
        }
       buffer1[b]= '\0';
        Client client = it->second;
        it->second._requestBuffer += buffer1;
        delete[] buffer1;
        pos1 = body.find("\r\n",pos2);
      //  std::cout <<"pos 1--->"<< pos1<< "pos 2--->"<< pos2 << std::endl; 
        std::cout <<   it->second._requestBuffer << std::endl;
    }
    std::cout << "b = "<< b<< std::endl;
    if (it->second.body_complete == true) {
        //std::cout << " ana hna salitttt\n";
}
else {
   

}
}


    // while (r.get_body().size() < content_length)
    // {
    //     ssize_t bytes_received = recv(client_fd, buffer, sizeof(buffer), 0);
    //     if (bytes_received <= 0)
    //         break;
    //     //buffer[bytes_received] = '\0';
    //     r.get_body().append(buffer, bytes_received);
    //  //   std::cout << "body size: " << r.get_body().size() << " / " << content_length << std::endl;
    // }
    // std::cout << "Final body size: " << r.get_body() << std::endl;
    // std::cout << "Expected: " << content_length << std::endl;
return r;
}


request::requetetException::requetetException(const std::string &msg) :_msg(msg){}

request::requetetException::~requetetException() throw() {}


const char* request::requetetException::what() const throw()
{
    return (this->_msg).c_str();
}
int request::get_final_port(request &r)
{
    std::map<std::string, std::string> map = r.get_header();
	std::map<std::string, std::string>::iterator it;
	for(it = map.begin() ; it != map.end(); it++)
	{
		if(it->first == "Host")
		{
			int pos = it->second.find(":");
			std::string port = it->second.substr(pos+1, it->second.size());
			int final_port = std::atoi(port.c_str());
            return final_port;
		}
	}
    return 0;
}