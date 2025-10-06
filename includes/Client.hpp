#pragma once

#include <iostream>
#include <unistd.h>
#include <ctime>
//#include"Server.hpp"
//#include "Request.hpp"
#include "Response.hpp"

struct CgiInfo {
    int pipefd;
    int start;// initailiserrr 
    bool flag_rep;/// initailiserrrrr 
    pid_t pid;
};

class Client
{
    private:
        std::time_t lastActivity;
    public :
    int cgi_active;
    std::string QUERY_STRING;
  size_t  chunk_size;
 int recived;
 std::string litter_chunk;
 bool reading_size;
        int _fd;
        int statusCode;
        bool body_complete;
        size_t ContentLength_chnked;
        bool chnked;
        std::string body_chunked;
        bool start_sending;
        bool CgiSendHeader;
        bool cgi_has_problem;
        bool send_complete;
        bool Sending;
        bool file_opened;
        bool no_data;
        bool ResponseChunked;
        bool autoindex;
        bool has_cookie;
        bool Read;
        bool has_cgi;
        std::string cookies;
        std::string autoIndexBody;
        std::string statusMsg;
        std::string PostBody;
        std::string ReturnLocation;
        std::string CgiBody;
        std::string CGIPostBody;
        std::string sessionId;
        std::string filename;
        std::vector<std::string> sessions;
        size_t filesize;
        size_t size_send;
        size_t conf_i ;
        size_t bytesRead;

        std::map<int, CgiInfo> cgiMap;///// in this map add cgi exit status code and do not send cgi response until code status is 200 
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
        std::vector<std::string> getSession() const;
        int getFd() const;
        int getLastActivity() const;
        time_t CgiStartActivity;
        void updateActivity();
        void receiveRequest();
        void sendResponse();
int get_final_port();

};
