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
request::~request(){}

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

bool request::error_set(std::map<int, Client>& clients, request &r, int clientFd , ServerConfig& config)
{
    std::map<std::string , std::string>headers = clients[clientFd].get_header();
    size_t pos = 0;
    pos = clients[clientFd].path.find("..");
    if(clients[clientFd].HeaderEnd  > 8000)
    {
        clients[clientFd].response = Response::buildResponse(r, 431, "Request Header Fields Too Large", config.ErrorPages[431], clientFd, clients);
        return 0;
    }
    if(pos  != std::string::npos)
    {
         clients[clientFd].response = Response::buildResponse(r, 400, "Bad Request", config.ErrorPages[400], clientFd, clients);
        return 0;
    }
    if(clients[clientFd].method != "GET" && clients[clientFd].method != "POST" && clients[clientFd].method != "DELETE")
    {
        clients[clientFd].response = Response::buildResponse(r, 501, "Not Implemented", config.ErrorPages[501], clientFd, clients);
        return 0;
    }
    if(clients[clientFd].version != "HTTP/1.1" && clients[clientFd].version != "HTTP/1.0" )
    {
        clients[clientFd].response = Response::buildResponse(r, 505, "HTTP Version Not Supported", config.ErrorPages[505], clientFd, clients);
        return 0;
    }
    if(headers.find("Host") == headers.end())	
    {
        std::cerr << "inside if statement of post\n";

        clients[clientFd].response = Response::buildResponse(r, 400, "Bad Request", config.ErrorPages[400], clientFd, clients);
        return 0;
    }
    if(clients[clientFd].method.empty() || clients[clientFd].path.empty())
    {
        clients[clientFd].response = Response::buildResponse(r, 400, "Bad Request", config.ErrorPages[400], clientFd, clients);
        return 0;
    }
    if(clients[clientFd].method == "POST")
    {
        std::map<std::string , std::string>::iterator ptr = headers.find("Content-Length"); 
        if(ptr == headers.end() && headers["Transfer-Encoding"] != "chunked" && clients[clientFd].method == "POST")
        {
            clients[clientFd].response = Response::buildResponse(r, 411,  "Length Required", config.ErrorPages[411], clientFd, clients);
            return 0;
        }
        if(ptr != headers.end())
        {
            if(!is_valid_content_length(ptr->second))
            {
                clients[clientFd].response = Response::buildResponse(r, 411,  "Length Required", config.ErrorPages[411], clientFd, clients);
                return 0;
            }
        }
        // if(clients[clientFd].chnked && clients[clientFd].body_chunked.size() >static_cast<size_t>(config[clients[clientFd].conf_i].client_max_body_size ) )
	    // {
        //     clients[clientFd].response= Response::buildResponse(r, 413, "Payload Too Large",config[clients[clientFd].conf_i].ErrorPages[413], clientFd, clients);
        //     return 0;
	    // }
	    // if (clients[clientFd].PostBody.size() >static_cast<size_t>( config[clients[clientFd].conf_i].client_max_body_size ) && !client[clientFd].chnked)
	    // {
		// //send_response(clientFd, 413, "Payload Too Large", load_html_file("www/413.html"));
		//     clients[clientFd].response= Response::buildResponse(r, 413, "Payload Too Large",config[clients[clientFd].conf_i].ErrorPages[413], clientFd, clientobj);
		//     return ;
	    // }
        }
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
 request::request() {
    reponse_status = 0;
    body = "";
    method = "" ;
    path = "";
    body_chnked = "";
    fullUploadpath = "";
    ContentType = "";
    ContentLength = 0;
 }

void request::set_method(std::string m) {method= m;}

void request::set_path(std::string p){path = p;}

void request::set_vergion(std::string v) {version= v;}

void request::set_header(std::string key, std::string value){
    map[key] = value;}

std::string request::get_method(){return method;}

std::string request::get_version(){return version;}

std::string request::get_path(){return path;}

std::map<std::string,std::string>& request::get_header()   
{
    return map;
}
void request::set_body(std::string& b){body = b;}

std::string& request::get_body(void){return body ;}


std::vector<std::string> parseCookies(const std::string &cookieHeader) 
{
    std::vector <std::string> cookies;
    std::istringstream ss(cookieHeader);
    std::string token;
    while (std::getline(ss, token, ';')) 
    {
        size_t pos = token.find('=');
        if (pos != std::string::npos) 
        {
            //std::string key = token.substr(0, pos);
            std::string value = token.substr(pos + 1);
            // remove spaces
            //key.erase(0, key.find_first_not_of(" "));
            cookies.push_back (value);
        }
    }
    return cookies;
}

std::string findCookies(const std::string &cookieHeader) 
{
    std::vector <std::string> cookies;
    std::istringstream ss(cookieHeader);
    std::string token;
    if(std::getline(ss, token, ';')) 
    {
        size_t pos = token.find('=');
        if (pos != std::string::npos) 
        {
            return(token.substr(pos + 1));
        }
    }
    return NULL;
}

request& request::parseRequest(std::map<int, Client>& clientobj, EpollManager &epollManager, request &r, int clientFd)
{
    Server s; 
    if (clientobj[clientFd].cgiMap[clientFd].pipefd != -1)
    {
        // std::cout << "should not be parsed it's a pipe event client\n";
        return r;
    }
    char buffer [8000] = {0};
    // std::cout << "\n\n\n-----Content in the buffer is: " << buffer << "-------\n\n";
    ssize_t bytes_received = recv(clientFd, buffer, sizeof(buffer), 0);
    // std::cout << "\n\n\n-----Content in the buffer is: " << buffer << "-------\n\n";
    // std::cout << "the buffer size is: " << sizeof(buffer) << std::endl;
    // usleep(10000);
    if ( bytes_received <= 0)
    {

        if (errno != EAGAIN && errno != EWOULDBLOCK)
        {
            s.closeConnection(clientFd, epollManager);
            throw requetetException("❌ recv failed: ");
        }
    }
    clientobj[clientFd].PostBody.append(buffer, bytes_received);
    //std::cout << "Postbody is sizzzzzzzzz: " << clientobj[clientFd].PostBody.size() << "*************<<<\n\n";
    // std::cout << "body complete is: " << clientobj[clientFd].body_complete << "[[[[[[]]]]]]\n\n";
    //std::cout << "in this socket file number: " << clientFd << "=> size in header: " << clientobj[clientFd].ContentLength << " and size in body is: " << clientobj[clientFd].PostBody.size() << std::endl;
    if (clientobj[clientFd].PostBody.find("\r\n\r\n") != std::string::npos && clientobj[clientFd].header_complete == 0 )
    {
        clientobj[clientFd].HeaderEnd = clientobj[clientFd].PostBody.find("\r\n\r\n");
        std::string headers = clientobj[clientFd].PostBody.substr(0, clientobj[clientFd].HeaderEnd );
        clientobj[clientFd].PostBody = clientobj[clientFd].PostBody.substr(clientobj[clientFd].HeaderEnd  + 4);
        std::istringstream iss(headers);
        std::string methode , path ,version, line;
        std::getline(iss, line,  '\r');
        iss.ignore();
        std::istringstream line_stream(line);
        line_stream >>  methode >> path >> version;
        if (methode.empty() || path.empty() || version.empty())
        {
            clientobj[clientFd].header_complete = 1;
            r.method = "";
            clientobj[clientFd].method = "";
            return r;
        }
        clientobj[clientFd].method = methode;
        size_t pos1;
        if((pos1 = path.find("?") )!= std::string::npos)
        {
            clientobj[clientFd].path = path.substr(0, pos1);
            clientobj[clientFd].QUERY_STRING = path.substr(pos1+1, path.size()- pos1);
        }
        else    
             clientobj[clientFd].path = trim1(path);
        clientobj[clientFd].version = version;
        // r.set_method(method);
        // r.set_path(path);
        // r.set_vergion(version);
        while(std::getline(iss, line, '\r') && !line.empty())
        {
            // std::cout << "line is: " << line << std::endl;
            iss.ignore();
            size_t pos = line.find(":");
            if(pos != std::string::npos)
            {
                std::string key = line.substr(0,pos);
                key = trim1(key);
                std::string value = line.substr(pos+1, line.size());
                value = trim1(value);
                clientobj[clientFd].set_header(key,value);
                r.set_header(key,value);
            }
        }
        std::map<std::string, std::string>::iterator iterator;
        iterator = clientobj[clientFd].get_header().begin();
        int count = 0;
        while (iterator != clientobj[clientFd].get_header().end())
        {
            if(iterator->first == "Transfer-Encoding" && iterator->second == "chunked")
            {
                clientobj[clientFd].chnked = 1;
            }
            else  if (iterator->first == "Content-Length")
            {
                //   std::cout << "welommmmmmmmmmm\n";
                count++;
                std::stringstream ss(iterator->second);
                ss >> clientobj[clientFd].ContentLength;
                // const unsigned long max_body_size = 1024 * 1024; // 1 Mo
                // if (r.ContentLength > max_body_size)
                // {
                    //     std::cout << "Problem here \n";
                    //     clientobj[clientFd].response = Response::buildResponse(r, 413, "Payload Too Large", "www/413.html", clientFd, clientobj);
                    //     // send_response(clientFd, 413, "Payload Too Large", load_html_file("www/413.html"));
                    //     return r;
                    // }
            }
            if (iterator->first == "Content-Type")
                clientobj[clientFd].ContentType = iterator->second;
            if (iterator->first == "Cookie")
            {
                clientobj[clientFd].cookies = iterator->second;
                std::string id = findCookies(clientobj[clientFd].cookies);
                std::cout << "Session ID: " << id << std::endl;
                clientobj[clientFd].has_cookie = 1;
            }
            iterator++;
        }
        if(count == 0 && !clientobj[clientFd].chnked)
            clientobj[clientFd].body_complete = 1;
        clientobj[clientFd].header_complete = 1;
    }
    if (clientobj[clientFd].header_complete)
    {
        r.set_method(clientobj[clientFd].method);
        r.set_path(clientobj[clientFd].path);
        r.set_vergion(clientobj[clientFd].version);
        if(clientobj[clientFd].chnked == 1)
        {
             while(1) { 
                
                 std::string chunk_string; 
                size_t pos2 =0;
                pos2 = clientobj[clientFd].PostBody.find("\r\n");
               if (pos2 == std::string::npos) { 
                    std::cout << "i m here\n";
                    break;
            } 
                std::string a = clientobj[clientFd].PostBody.substr(0,pos2);
                clientobj[clientFd].chunk_size = std::strtol(a.c_str(),NULL,16);
                if(clientobj[clientFd].chunk_size==0)
                {
                   clientobj[clientFd].body_complete = 1;
                   break;
                }
                    
                     if( clientobj[clientFd].PostBody.size() < pos2 + 2 + clientobj[clientFd].chunk_size)
                     {
                        break;
                     }
                    clientobj[clientFd].PostBody.erase(0, pos2+2);
                    chunk_string = clientobj[clientFd].PostBody.substr(0,clientobj[clientFd].chunk_size);
                    clientobj[clientFd].body_chunked.append(chunk_string);
                    clientobj[clientFd].PostBody.erase(0, clientobj[clientFd].chunk_size +2);
         } 
        }
        if (clientobj[clientFd].ContentLength == clientobj[clientFd].PostBody.size() || clientobj[clientFd].method == "GET")
        {
            std::cout << "haniiiiiii------------>\n";
            clientobj[clientFd].body_complete = 1;
            if (clientobj[clientFd].method == "POST")
            {
                std::cout << "\nReading Post body is Done ✅\n";
                clientobj[clientFd].send_complete = 1;
            }
            // std::cout << "THe body has been recieved";
        }
    }
    
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
