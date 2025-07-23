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
#define e405 "405 Method Not Allowed"
#define e505 "505 HTTP Version Not Supported"
#define e400  "400 Bad Request"
#define e413 "Payload Too Large"
#define e404 "Page not found"


class request{
   private :
   std::string body;
    std::string method;
    // std::string path;
    std::string version;
    std::string path;
    std::map<std::string, std::string> map;
    public :
    void set_method(std::string m) {method= m;}
    void set_path(std::string p){path = p;}
   void set_vergion(std::string v) {version= v;}
   void set_header(std::string key, std::string value){
    map[key] = value;}
   std::string get_method(){return method;}
   std::string get_version(){return version;}
    std::string get_path(){return path;}
   std::map<std::string,std::string>get_header() {
    return map;
   }
   void set_body(std::string& b){body = b;}
  std::string get_body(void){return body ;}
};

int parseRequest(std::string buffer, request &r);

#endif