#ifndef REQUEST_HPP
#define REQUEST_HPP
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <sstream>
#include<map>
#include<string>
#include<cstdlib>
#include "EpollManager.hpp"
#define e405 "405 Method Not Allowed"
#define e505 "505 HTTP Version Not Supported"
#define e400  "400 Bad Request"
#define e413 "Payload Too Large"
#include"Server.hpp"
#include"Client.hpp"
#include <cerrno>   // pour errno, EAGAIN, EWOULDBLOCK
#include <cstdio> 
class EpollManager;



#define e404 "Page not found"


class request{
    public:
    request& operator=(const request& other);
    request(request const &ref);
    request();
   int reponse_status;
  
   std::string body;
    std::string method;
    std::string path;
    std::string body_chnked;
    std::string ContentType;
    std::string ContentLength;
    request& parseRequest(std::map<int, Client>& clientobj , EpollManager &epollManager, request &r, int clientFd);
    int get_final_port(request &r);
    std::string version;
    std::map<std::string, std::string> map;
    void set_method(std::string m);
    void set_path(std::string p);
   void set_vergion(std::string v);
   void set_content(std::string content);
   void set_header(std::string key, std::string value);
   std::string get_method();
   std::string get_version();
   std::string get_path();
   std::map<std::string,std::string>& get_header();
   void set_body(std::string& b);
  std::string& get_body(void);
  bool error_set(std::map<int, Client>& clientobj, request &r, int clientfd);
 class requetetException : public std::exception 
  {
    private:
      std::string _msg;
    public:
      requetetException(const std::string &msg);    
      virtual ~requetetException() throw();    
      virtual const char* what() const throw();
  };
};


#endif