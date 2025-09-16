#pragma once

#include <iostream>
#include <unistd.h>
#include <ctime>
//#include"Server.hpp"
//#include "Request.hpp"
#include "Response.hpp"

struct CgiInfo {
    int pipefd;
    pid_t pid;
};

class Client
{
    private:
        std::time_t lastActivity;
    public :
  size_t  chunk_size;
 int recived;
 std::string litter_chunk;
 bool reading_size;
        int _fd;
        bool body_complete;
        size_t ContentLength_chnked;
        bool chnked;
        std::string body_chunked;
        bool start_sending;
        bool send_complete;
        bool Sending;
        bool file_opened;
        bool no_data;
        bool ResponseChunked;
        bool autoindex;
        bool has_cookie;
        bool has_cgi;
        std::string cookies;
        std::string autoIndexBody;
        std::string PostBody;
        std::string CgiBody;
        size_t CgibytesRead;
        size_t filesize;
        size_t size_send;
        size_t conf_i ;

        std::map<int, CgiInfo> cgiMap;
        std::map<int, std::string> GetpathMap;
        
        
        std::map<std::string, std::string> map;
        std::string ContentType;
        size_t ContentLength;
        bool header_complete ;
        bool create_file;
        std::string header;
        std::string method;
        std::string path;
        std::string version;
        Response response;

        Client();
        Client(int fd);
        ~Client();
        void set_header(std::string key, std::string value);
        std::map<std::string,std::string>& get_header();
        int getFd() const;
        int getLastActivity() const;
        time_t CgiStartActivity;
        void updateActivity();
        void receiveRequest();
        void sendResponse();
};
