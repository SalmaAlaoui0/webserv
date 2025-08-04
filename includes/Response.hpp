#ifndef RESPONSE_HPP
#define RESPONSE_HPP
#include<iostream>
class reponse{
    public : 
    int reponse_status;
    std::string response_body;

};

std::string RequestResponse(int clientFd, std::string filePath, std::string resCode);


#endif