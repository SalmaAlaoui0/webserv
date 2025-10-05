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

bool request::error_set(std::map<int, Client>& clients, request &r, int clientFd , ServerConfig& config)
{
    std::cout  << " jittttttttttt\n\n";
    std::map<std::string , std::string>headers = clients[clientFd].get_header();
    if(clients[clientFd].method != "GET" && clients[clientFd].method != "POST" && clients[clientFd].method != "DELETE")
    {
        
        std::cout << "Method is: " << r.get_method() << std::endl; /// telnet 127.0.0.1 8080 there is a problem here the get method does not return the method but the path if we're using telnet as a client try it!
        clients[clientFd].response = Response::buildResponse(r, 405, "Method Not Allowed", config.ErrorPages[405], clientFd, clients);
        return 0;
    }
    if(clients[clientFd].method == "POST")/// 5ass n3awd les error dyl post 
    {
        // std::map<std::string , std::string>::iterator ptr = headers.find("Content-Length"); 
        // if(ptr != headers.end())
        // {
		// 	std::string a = ptr->second;
		// 	unsigned long b = std::atoi(a.c_str());
		// 	if(b < 0 || (b != r.get_body().size()))
        //     {
        //         std::cout << "b is: " << b << " and r.getbodysize is: " << r.get_body().size() << std::endl;
        //         clients[clientFd].response = Response::buildResponse(r, 500, "Internal Server Error", config.ErrorPages[500], clientFd, clients);
        //         //send_response(clientfd, 400, "Bad Request", load_html_file("www/400.html"));

        //         return 0;
        //     }
        // }
        // else
        // {
        //     //send_response(clientfd, 400, "Bad Request", load_html_file("www/400.html"));
        //     std::cout << " khrjattttttt4\n";
        //     clients[clientFd].response = Response::buildResponse(r, 500, "Internal Server Error", config.ErrorPages[500], clientFd, clients);

        //     return 0;
        // }
    }
	if(clients[clientFd].version != "HTTP/1.1")
    {
        std::cout << "here internalllll in version\n";
        clients[clientFd].response = Response::buildResponse(r, 505, "HTTP Version Not Supported", config.ErrorPages[505], clientFd, clients);
        return 0;
    }
    // // debug
    // std::cerr << "DEBUG: clientFd=" << clientFd << " config addr=" << &config
    //       << " ErrorPages.size=" << config.ErrorPages.size() << std::endl;

    // for (std::map<int, std::string>::const_iterator it = config.ErrorPages.begin(); it != config.ErrorPages.end(); ++it)
    // {
    //     std::cerr << "  page: " << it->first << " => " << it->second << std::endl;
    // }
    // //
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
    // const unsigned long max_body_size = 1024 * 1024; // 1 Mo
    // if (r.get_body().size() > max_body_size)
    // {
    //     std::cout << "In some conditions waiting for the faith and the error page is:" << config.ErrorPages[413] << " and the clientFd is: " << clientFd << " \n";
    //     clients[clientFd].response = Response::buildResponse(r, 413, "Payload Too Large", config.ErrorPages[413], clientFd, clients);
    //     return 0;
    // }
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
    // std::cout << "Postbody is: " << clientobj[clientFd].PostBody << "*************<<<\n\n";
    // std::cout << "body complete is: " << clientobj[clientFd].body_complete << "[[[[[[]]]]]]\n\n";
    //std::cout << "in this socket file number: " << clientFd << "=> size in header: " << clientobj[clientFd].ContentLength << " and size in body is: " << clientobj[clientFd].PostBody.size() << std::endl;
    if (clientobj[clientFd].PostBody.find("\r\n\r\n") != std::string::npos && clientobj[clientFd].header_complete == 0 )
    {
        size_t HeaderEnd = clientobj[clientFd].PostBody.find("\r\n\r\n");
        std::string headers = clientobj[clientFd].PostBody.substr(0, HeaderEnd);
        clientobj[clientFd].PostBody = clientobj[clientFd].PostBody.substr(HeaderEnd + 4);
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
        size_t pos;
        if( ( pos = path.find("cgi-bin")) != std::string::npos)
        {
             clientobj[clientFd].cgi_active = true;
        }
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
            std::cout << "line is: " << line << std::endl;
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
        while (iterator != clientobj[clientFd].get_header().end())
        {
            if(iterator->first == "Transfer-Encoding" && iterator->second == "chunked")
            {
                clientobj[clientFd].chnked = 1;
            }
            else  if (iterator->first == "Content-Length")
            {
                //   std::cout << "welommmmmmmmmmm\n";
                
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
            {
                clientobj[clientFd].ContentType = iterator->second;
            }
            if (iterator->first == "Cookie")  //zadt cookies 
            {
                clientobj[clientFd].cookies = iterator->second;
                std::string id = findCookies(clientobj[clientFd].cookies);
                //if (s.getSession().find(id) != s.getSession().end())
                std::cout << "Session ID: " << id << std::endl;
                clientobj[clientFd].has_cookie = 1;
            }
            iterator++;
        }
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
