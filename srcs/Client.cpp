#include "../includes/Client.hpp"

Client::Client()
: lastActivity(std::time(NULL)),
  cgi_active(0),
  QUERY_STRING(""),
  chunk_size(0),
  recived(0),
  litter_chunk(""),
  reading_size(false),
  _fd(-1),
  statusCode(0),
  body_complete(false),
  ContentLength_chnked(0),
  chnked(false),
  body_chunked(""),
  start_sending(false),
  CgiSend(false),
  cgi_has_problem(false),
  send_complete(false),
  Sending(false),
  file_opened(false),
  no_data(false),
  ResponseChunked(false),
  autoindex(false),
  has_cookie(false),
  Read(false),
  has_cgi(false),
  cookies(""),
  autoIndexBody(""),
  statusMsg(""),
  PostBody(""),
  ReturnLocation(""),
  CgiBody(""),
  CGIPostBody(""),
  sessionId(""),
  filename(""),
  sessions(),
  filesize(0),
  size_send(0),
  conf_i(0),
  bytesRead(0),
  HeaderEnd(0),
  cgiMap(),
  GetpathMap(),
  map(),
  ContentType(""),
  ContentLength(0),
  header_complete(false),
  create_file(false),
  header(""),
  method(""),
  path(""),
  version(""),
  response(),
  CgiStartActivity(0)
{
        updateActivity();
    body_complete = false;
}

Client::Client(int fd)
: lastActivity(std::time(NULL)),
  cgi_active(0),
  QUERY_STRING(""),
  chunk_size(0),
  recived(0),
  litter_chunk(""),
  reading_size(false),
  _fd(fd),
  statusCode(0),
  body_complete(false),
  ContentLength_chnked(0),
  chnked(false),
  body_chunked(""),
  start_sending(false),
  CgiSend(false),
  cgi_has_problem(false),
  send_complete(false),
  Sending(false),
  file_opened(false),
  no_data(false),
  ResponseChunked(false),
  autoindex(false),
  has_cookie(false),
  Read(false),
  has_cgi(false),
  cookies(""),
  autoIndexBody(""),
  statusMsg(""),
  PostBody(""),
  ReturnLocation(""),
  CgiBody(""),
  CGIPostBody(""),
  sessionId(""),
  filename(""),
  sessions(),
  filesize(0),
  size_send(0),
  conf_i(0),
  bytesRead(0),
  HeaderEnd(0),
  cgiMap(),
  GetpathMap(),
  map(),
  ContentType(""),
  ContentLength(0),
  header_complete(false),
  create_file(false),
  header(""),
  method(""),
  path(""),
  version(""),
  response(),
  CgiStartActivity(std::time(NULL))
{
    updateActivity();
}

Client::~Client()
{
     //close (_fd);
}

int Client::get_final_port()
{
    std::map<std::string, std::string> map = this->map;
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

void Client::set_header(std::string key, std::string value){
    map[key] = value;}

std::map<std::string,std::string>& Client::get_header()   {
return map;
}

 std::vector<std::string> Client::getSession() const  {
return sessions;
}


// void Client::set_header(std::string key, std::string value){
//     map[key] = value;}


// std::map<std::string,std::string>& Client::get_header()   {
//     return map;
//    }

int Client::getFd() const
{
    return(_fd);
}

int Client::getLastActivity() const
{
    return(lastActivity);
}

void Client::updateActivity()
{
    lastActivity = std::time(NULL);
}