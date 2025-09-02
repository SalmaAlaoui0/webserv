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

bool request::error_set(std::map<int, Client>& clients, request &r, int clientFd , ServerConfig &config)
{
    std::map<std::string , std::string>headers = r.get_header();
    //std::cout << "helllllo there method is: " << clients[clientFd].method << "that's itttttttt\n";
    std::cout << "helllllo there method is$$$$$$$$$: " << r.get_method() << "that's itttttttt\n";
    std::map<int, Client> :: iterator it = clients.find(clientFd);
    if(it == clients.end())
    {
        std::cerr << "❌ clientFd " << clientFd << " not found\n";

        return 0;
    }
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
	if(headers.find("Host") == headers.end())	
    {
        clients[clientFd].response = Response::buildResponse(r, 400, "Bad Request", config.ErrorPages[400], clientFd, clients);
        return 0;
    }
    if(clients[clientFd].method.empty() || clients[clientFd].path.empty())
    {
        clients[clientFd].response = Response::buildResponse(r, 400, "Bad Request", config.ErrorPages[400], clientFd, clients);
        return 0;
    }
    const unsigned long max_body_size = 1024 * 1024; // 1 Mo
    if (r.get_body().size() > max_body_size)
    {
        clients[clientFd].response = Response::buildResponse(r, 413, "Payload Too Large", config.ErrorPages[413], clientFd, clients);
        return 0;
    }
    return 1;
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

request& request::parseRequest(std::map<int, Client>& clientobj, EpollManager &epollManager, request &r, int clientFd)
{
    std::cout << "hiiiiiiiii******************************\n";
    Server s;
    char buffer [1024] = {0};
    ssize_t bytes_received = recv(clientFd, buffer, sizeof(buffer), 0);
    if ( bytes_received == -1)
    {
        if (errno != EAGAIN && errno != EWOULDBLOCK)
        {
            s.closeConnection(clientFd, epollManager);
            throw requetetException("❌ recv failed: ");
        }
    }
    r.body.append(buffer, bytes_received);
    std::cout << "in this socket file number: " << clientFd << "=> size in header: " << r.ContentLength << " and size in body is: " << r.body.size() << std::endl;
    // std::cout << "the size in header is: " << r.ContentLength << "and the body is: " << r.get_body().size() << std::endl;
    if (r.body.find("\r\n\r\n") != std::string::npos)
    {
        size_t HeaderEnd = r.body.find("\r\n\r\n");
        std::string headers = r.body.substr(0, HeaderEnd);
        r.body = r.body.substr(HeaderEnd + 4);
        // std::cout << "\n\n" << headers << "\n\n";///
        std::istringstream iss(headers);
        std::string methode , path ,version, line;
        std::getline(iss, line,  '\r');
        iss.ignore();
        std::istringstream line_stream(line);
        line_stream >>  methode >> path >> version;
        r.set_method(method);
        r.set_path(path);
        r.set_vergion(version);
        clientobj[clientFd].method = methode;
        clientobj[clientFd].path = path;
        clientobj[clientFd].version = version;
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
        std::map<std::string, std::string>::iterator iterator;
        iterator = r.get_header().begin();
        while (iterator != r.get_header().end())
        {
            if (iterator->first == "Content-Length")
            {
                std::stringstream ss(iterator->second);
                ss >> r.ContentLength;
                // const unsigned long max_body_size = 1024 * 1024; // 1 Mo
                // if (r.ContentLength > max_body_size)
                // {
                //     send_response(clientFd, 413, "Payload Too Large", load_html_file("www/413.html"));
                //     return r;
                // }
            }
            if (iterator->first == "Content-Type")
            {
                r.ContentType = iterator->second;
            }
            iterator++;
        }
        clientobj[clientFd].header_complete = 1;
    }
    if (clientobj[clientFd].header_complete)
    {
        if (r.ContentLength > r.body.size() && clientobj[clientFd].body_complete == 0)
        {
            if (r.ContentLength == r.body.size() || clientobj[clientFd].method == "GET")
            {
                clientobj[clientFd].body_complete = 1;
                if (clientobj[clientFd].method == "POST")
                    clientobj[clientFd].send_complete = 1;
                r.set_method(clientobj[clientFd].method);
                r.set_path(clientobj[clientFd].path);
                r.set_vergion(clientobj[clientFd].version);
                // std::cout << "1THe body has been recieved";
            }
        }
        else if (r.ContentLength == r.body.size() || clientobj[clientFd].method == "GET")
        {
            clientobj[clientFd].body_complete = 1;
            if (clientobj[clientFd].method == "POST")
                clientobj[clientFd].send_complete = 1;
            r.set_method(clientobj[clientFd].method);
            r.set_path(clientobj[clientFd].path);
            r.set_vergion(clientobj[clientFd].version);
            // std::cout << "THe body has been recieved";

        }
    }
    // std::cout << "\n\n\nHEREEEEEEEEEEEEEEEEEEEEEEEE"  << "---------------\n\n\n";
    // std::string raw_request(buffer);//, bytes_received);
    // size_t pos = raw_request.find("\r\n\r\n");
    // // std::cout << "BUFFER IIIIIIIIS: " << buffer << ":::::::::" << std::endl;
    // std::cout << "size of body is: " << r.get_body().size() << std::endl;
    // if (pos != std::string::npos)
    // {
    //     pos += 4;
    //     std::string initial_body = raw_request.substr(pos);
    //     r.get_body().append(initial_body);
    //     std::cout << "appended is: ^^^^^^^" << r.get_body() << "^^^^^^^^^\n";
    // }
  
//     if (r.get_header()["Transfer-Encoding"] == "chunked")
//     {
//         std::cout << "chunkedddddddd\n";
//         std::string body = r.get_body();
//         char *buffer1;
//         int b = 0;
//         size_t pos2 = 0;
//         size_t pos1 = body.find("\r\n",pos2);

//         while( pos1 != std::string::npos)
//         {
//         std::string a = body.substr(pos2,pos1);
//         b = std::strtol(a.c_str(),NULL,16);
//         if(b <= 0)
//         {
//             it->second.body_complete = true;
//             break;
//         }
//         buffer1 = new char[1024];
//       //std::cout << "a ==>" << a<< std::endl;
//       //std::cout << "b ==>" << b<< std::endl;
//      int i =0;
//       pos1 += 2;
//       std::string c = body.substr(pos1,b);

//     pos2 = b +pos1 + 2;
     
//       while(i < b)
//       {
//           buffer1[i] = c[i];
//           i++;
//         }
//        buffer1[b]= '\0';
//         Client client = it->second;
//         it->second._requestBuffer += buffer1;
//         delete[] buffer1;
//         pos1 = body.find("\r\n",pos2);
//       //  std::cout <<"pos 1--->"<< pos1<< "pos 2--->"<< pos2 << std::endl; 
//         std::cout <<   it->second._requestBuffer << std::endl;
//     }
//     std::cout << "b = "<< b<< std::endl;
//     if (it->second.body_complete == true) {
//         //std::cout << " ana hna salitttt\n";
// }
// else {
   

// }
// }


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