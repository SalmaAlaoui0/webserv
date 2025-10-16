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
#include"Server.hpp"
#include"Client.hpp"
#include<sstream>
#include <cerrno> 
#include <cstdio>

class EpollManager;
class Client;
class request
{
    public:
    request();
    request(request const &ref);
    request& operator=(const request& other);
    ~request();
    int reponse_status;
    std::string body;
    std::string method;
    std::string path;
    std::string body_chnked;
    std::string fullUploadpath;
    std::string ContentType;
    size_t ContentLength;
    request& parseRequest(std::map<int, Client>& clientobj , EpollManager &epollManager, request &r, int clientFd,std::vector<ServerConfig> &_configs);
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
  bool error_set(std::map<int, Client>& clients, int clientFd ,  ServerConfig& configs, std::vector<ServerConfig> &_configs);
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
 std::string trim1(std::string &s);

#endif