#pragma once

#include <iostream>
#include <unistd.h>
#include <ctime>
//#include"Server.hpp"
//#include "Request.hpp"
#include "Response.hpp"

class Client
{
    private:
        std::time_t lastActivity;
    public :
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
        // std::ifstream file;
        // int fd;
        std::string autoIndexBody;
        std::string PostBody;
        size_t filesize;
        size_t size_send;
        size_t conf_i ;

        std::map<std::string, std::string> map;
        std::string ContentType;
        size_t ContentLength;
        // size_t filesize;
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
        void updateActivity();
        void receiveRequest();
        void sendResponse();
};
