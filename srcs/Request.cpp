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

bool request::error_set(std::map<int, Client>& clientobj, request &r, int clientfd)
{
    std::map<std::string , std::string>headers = r.get_header();
    std::cout << "helllllo there method is: " << clientobj[clientfd].method << "that's itttttttt\n";
    std::cout << "helllllo there method is: " << r.get_method() << "that's itttttttt\n";
    if(clientobj[clientfd].method != "GET" && clientobj[clientfd].method != "POST" && clientobj[clientfd].method != "DELETE")
    {
        std::cout << "Method is: " << r.get_method() << std::endl; /// telnet 127.0.0.1 8080 there is a problem here the get method does not return the method but the path if we're using telnet as a client try it!
        send_response(clientfd, 405, "Method Not Allowed", load_html_file("www/405.html"));
        return 0;
    }
    if(r.get_method() == "POST")
    {
        std::map<std::string , std::string>::iterator ptr = headers.find("Content-Length"); 
        if(ptr != headers.end())
        {
			std::string a = ptr->second;
			unsigned long b = std::atoi(a.c_str());
			if(b < 0 )//|| (b != r.get_body().size())) // 
            {
                std::cout << "b is: " << b << " and r.getbodysize is: " << r.get_body().size() << std::endl;
                send_response(clientfd, 400, "Bad Request", load_html_file("www/400.html"));
                return 0;
            }
        }
        else
        {
            std::cout << "22222222222222222----\n";
            send_response(clientfd, 400, "Bad Request", load_html_file("www/400.html"));
            std::cout << "helllllllo the world\n";
            return 0;
        }
    }
	if(clientobj[clientfd].version != "HTTP/1.1")
    {
        send_response(clientfd, 505, "HTTP Version Not Supported", load_html_file("www/505.html"));
        return 0;
    }
	if(headers.find("Host") == headers.end())	
    {
        std::cout << "3333333333333333\n";
        send_response(clientfd, 400, "Bad Request", load_html_file("www/400.html"));
        return 0;
    }
    if(r.get_method().empty() || r.get_path().empty())
    {
        std::cout << "the method and path are: " << r.get_method() << "---" << r.get_path() << std::endl;;
        send_response(clientfd, 400, "Bad Request", load_html_file("www/400.html"));
        return 0;
    }
    const unsigned long max_body_size = 1024 * 1024; // 1 Mo
    if (r.get_body().size() > max_body_size)
    {
        send_response(clientfd, 413, "Payload Too Large", load_html_file("www/413.html"));
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
    Server s;
    //std::vector<char> buffer;
    char buffer [1024] = {0};
    // Client &client = clientobj[clientFd];
    // std::map<int,Client>::iterator it = clientobj.begin();
    // std::string RecievedText;
    // std::string RecievedHeader;
    std::ofstream out;
    std::string RecievedBody;
    // std::cout << "Hello World!" << std::endl;
    ssize_t bytes_received = recv(clientFd, buffer, sizeof(buffer), 0);
    // buffer[bytes_received] = '\0';
    // std::cout << "==================ENTERED========================\n";
    if ( bytes_received == -1)
    {
        if (errno != EAGAIN && errno != EWOULDBLOCK)
        {
            // s.closeConnection(it->first, epollManager);
            s.closeConnection(clientFd, epollManager);
            throw requetetException("❌ recv failed: ");
        }
    }
    clientobj[clientFd]._requestBuffer.append(buffer, bytes_received);
    r.body.append(buffer, bytes_received);
    if (clientobj[clientFd].create_file == 1)
    {
        out .write(buffer, 1024);
    }
    // std::cout << "***recieved is :" << clientobj[clientFd]._requestBuffer << "yes this is the recieved***\n";
    if (clientobj[clientFd]._requestBuffer.find("\r\n\r\n") != std::string::npos)
    {
        // std::cout << "hello world\n";
        size_t HeaderEnd = clientobj[clientFd]._requestBuffer.find("\r\n\r\n");
        std::string headers = clientobj[clientFd]._requestBuffer.substr(0, HeaderEnd);
        // std::cout << "hello world HEREEEERERE\n";
        clientobj[clientFd]._requestBuffer = clientobj[clientFd]._requestBuffer.substr(HeaderEnd + 4);
        r.body = clientobj[clientFd]._requestBuffer;
        // std::cout << "%%%%%header is:" << RecievedHeader << "%%%%%\n\n";
        std::istringstream iss(headers);
        std::string methode , path ,version, line;
        std::getline(iss , line ,  '\r');
        iss.ignore();
        std::istringstream line_stream(line);
        line_stream >>  methode >> path >> version;
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
            //std::cout << key<<std::endl;
            std::string value = line.substr(pos+1, line.size());
            value = trim1(value);
        // std::cout << value<<std::endl;
            r.set_header(key,value);
            }
        }
        // std::cout << "we reached untill heereeeeee\n";
        std::map<std::string, std::string>::iterator iterator;
        iterator = r.get_header().begin();
        while (iterator != r.get_header().end())
        {
            if (iterator->first == "Content-Length")
            {
                r.ContentLength = iterator->second;
                clientobj[clientFd].contentLength = static_cast<size_t>(std::stoi(r.ContentLength));
                std::cout << iterator->first << "-------------------------first and second-> "<< iterator->second << std::endl;
            }
            if (iterator->first == "Content-Type")
            {
                r.ContentType = iterator->second;
                clientobj[clientFd].contentType = ContentType;
                std::cout << iterator->first << "-------------------------first and second-> "<< iterator->second << std::endl;
            }
            iterator++;
        }
        if (!clientobj[clientFd].create_file)
        {
            // 
            std::ostringstream filename;
        std::cout << "^^^^^" << r.ContentType << std::endl;
        int ext = r.ContentType.find('/');
        r.ContentType = r.ContentType.substr(ext + 1);
        filename << "home/salaoui/Desktop/webserv/www/upload" << "/" << rand() <<"."<< r.ContentType;
        std::cout << "****" << filename.str() << std::endl;
        std::ofstream out(filename.str().c_str(),std::ios::binary);
        if(!out)
        {
            std::cerr << "❌ Failed in parse to open file: " << filename.str() << std::endl;
            send_newresponse(clientFd, 500, "Internal Server Error", load_html_file("www/500.html"), r.ContentType);
            return r;
        }
        // std::cout << "❌❌❌❌❌❌❌❌❌❌body--------" << r.bo dy << "-----------" <<"\n\n";
        //out << r.body;
        // std::cout << "the body is; " << r.body.c_str();

        // std::cout << "✅✅✅✅✅✅✅✅✅✅" << r.body << std::endl;
        out .write(r.body.c_str(), r.body.size());

            clientobj[clientFd].create_file = 1;
        }
        clientobj[clientFd].header_complete = 1;
    }
    // else if ( bytes_received == 0)
    //     throw requetetException("❌ Client disconnected ");
    // else if(buffer[bytes_received] == '\0' && bytes_received == 1 )
    //     throw requetetException("Empty request or client closed");
    // std::cout << "\n\n\n-------------" << "---------------\n\n\n";
    if (clientobj[clientFd].header_complete)
    {
        // std::cout << "HHHHHHHHH- SO HEADER IS COMPLETE AND YAP THAT'S IT-HHHHHHHHH\n\n";
        // std::cout << "salaaaaaaaaam the request content lenght is: " << clientobj[clientFd].contentLength << " so and the client content lenght is: " << clientobj[clientFd]._requestBuffer.size() << "and here they are both\n\n";
        if (clientobj[clientFd].contentLength > clientobj[clientFd]._requestBuffer.size() && clientobj[clientFd].body_complete == 0)
        {
            std::cout << "\n\n\n11111111111111111" << "---------------\n\n\n";
            // std::cout << "!!!!!!Body is:" << clientobj[clientFd]._requestBuffer << "!!!!!!\n\n";
            // r.body.append(RecievedText, sizeof(RecievedText));
            // std::cout << "\nOkey just to make it clear--content-length is: " << clientobj[clientFd].contentLength << 
            // "-- and length of body recieved is--" << clientobj[clientFd]._requestBuffer.size() << "--\n\n";
            if (clientobj[clientFd].contentLength == clientobj[clientFd]._requestBuffer.size())
            {
                clientobj[clientFd].body_complete = 1;
                out.flush();
                out.close();
                // std::cout << "THe hoooole body has been recieved wanna see: =>";
                // std::cout << "\n\n^^^" << clientobj[clientFd]._requestBuffer << "^^^\n\n";
            }
        }
        else if (clientobj[clientFd].contentLength == clientobj[clientFd]._requestBuffer.size())
        {
            std::cout << "\n\n\n2222222222222222222" << "---------------\n\n\n";
            clientobj[clientFd].body_complete = 1;
            out.flush();
            out.close();
            // std::cout << "THe hoooole body has been recieved wanna see: =>";
            // std::cout << "\n\n^^^" << clientobj[clientFd]._requestBuffer << "^^^\n\n";
            r.set_method(clientobj[clientFd].method);
            r.set_path(clientobj[clientFd].path);
            r.set_vergion(clientobj[clientFd].version);
        }
        // std::cout << "\n\n\n-------------llll" << "---------------\n\n\n";
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