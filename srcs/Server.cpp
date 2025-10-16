/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: wzahir <wzahir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/08 15:25:50 by wzahir            #+#    #+#             */
/*   Updated: 2025/08/29 21:44:06 by wzahir           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "../includes/Server.hpp"
#include "../includes/Utils.hpp"
#include <csignal>
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define WHITE   "\033[37m"

static bool running = 1; 

void handle_sigint(int)
{
    running = 0;  
    std::cout << "\n🛑 SIGINT received, shutting down..." << std::endl;
}


int Server::creatServerSocket(const std::string &ip, int port)
{
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
        throw socketException("❌ Socket creation failed");
    if (fcntl(server_fd, F_SETFL, O_NONBLOCK) < 0) 
    {
        close(server_fd);
        throw socketException("❌ Failed to set non-blocking mode on socket");
    }
    if (fcntl(server_fd, F_SETFD, FD_CLOEXEC) < 0) 
    {
        close(server_fd);
        throw socketException("❌ Failed to set FD_CLOEXEC");
    }
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        close(server_fd);
        throw socketException("❌ setsockopt(SO_REUSEADDR) failed");
    }
    struct addrinfo hints;
    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags    = AI_PASSIVE;
    std::stringstream ss;
    ss << port;
    std::string portStr = ss.str();
    struct addrinfo *res;
    const char *host = ip.empty() ? NULL : ip.c_str();
    int status = getaddrinfo(host, portStr.c_str(), &hints, &res);
    if (status != 0) 
    {
        close(server_fd);
        throw socketException(std::string("❌ getaddrinfo: ") + gai_strerror(status));
    }
    if (bind(server_fd, res->ai_addr, res->ai_addrlen) < 0) 
    {
        freeaddrinfo(res);
        close(server_fd);
        throw socketException("❌ bind() failed");
    }
    if (listen(server_fd, SOMAXCONN) < 0) 
    {
        freeaddrinfo(res);
        close(server_fd);
        throw socketException("❌ listen() failed");
    }
    freeaddrinfo(res);

    std::cout << GREEN << "✅ Listening on " << CYAN << (ip.empty() ? "0.0.0.0" : ip)
              << RESET << ":" << YELLOW << port << RESET << std::endl;
    return server_fd;
}

void Server:: setupSockets()
{
    for(size_t i = 0; i < _configs.size(); i++)
    {
        if (_configs[i].port != 0)
        {
            const std::string &ip =_configs[i].host;
            int port = _configs[i].port;
            this->serverSockets.push_back(creatServerSocket(ip, port));
       }
   }
}

Server::Server(const std::vector<ServerConfig> &configs)
{
    this->_configs = configs;
    try
    {
        setupSockets();
    }
  catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        for (size_t i = 0; i < serverSockets.size(); ++i)
            close(serverSockets[i]);
        serverSockets.clear();
        throw;
    }
}

Server::Server() {}

Server::~Server()
{
    for (size_t i = 0; i < serverSockets.size(); ++i)
        close(serverSockets[i]);
}

std::vector<ServerConfig> Server::getConfig() const
{
    return this->_configs;
}

 std::vector<std::string> Server:: getSession() const
 {
    return sessions;
 }

void Server::handleRequest(int clientFd, request &r, std::map<int, Client> &clientobj, EpollManager &epoll)
{
    if (this->clients[clientFd]. conf_i == _configs.size()) 
    {
        clients[clientFd].response = Response::buildResponse(500, "Internal Server Error (no matching server)",_configs[this->clients[clientFd]. conf_i].ErrorPages[500], clientFd, clients, _configs);
        return;
    }
    if (clientobj[clientFd].method  == "/favicon.ico")
    {
        clients[clientFd].response = Response::buildResponse(404, "Not Found",_configs[this->clients[clientFd]. conf_i].ErrorPages[404], clientFd, clients, _configs);
        return;
    }
	if (clientobj[clientFd].method == "GET")
        handle_get_methode(r, this->_configs, clientFd, this->clients[clientFd]. conf_i, clientobj, epoll);
    else if(clientobj[clientFd].method== "POST")
        handle_post_methode(r, this->_configs, clientFd, this->clients[clientFd]. conf_i, clientobj, epoll);
    else if (clientobj[clientFd].method  == "DELETE")
		handle_delete_methode(r, this->_configs, clientFd, this->clients[clientFd]. conf_i, clientobj);
    else
    {
        clients[clientFd].response = Response::buildResponse(403, "forbidden : invalid method",_configs[this->clients[clientFd]. conf_i].ErrorPages[403], clientFd, clients, _configs);
		return;
    }
}

void Server::acceptNewClient(request &req, int serverFd, EpollManager &epollManager)
{
        struct sockaddr clientAddr;
        req.body.clear();
        req.path.clear();
        socklen_t clientLen = sizeof(clientAddr);
        int clientFd = accept(serverFd, (sockaddr *)&clientAddr , &clientLen);
        if (clientFd < 0) 
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                return;
            std::cerr<<"❌ accept failed\n";
        }
        if (fcntl(clientFd, F_SETFL, O_NONBLOCK) == -1) 
        {
            close(clientFd);
            std::cerr<<"❌ fcntl() failed\n";
        }
        if (fcntl(clientFd, F_SETFD, FD_CLOEXEC) == -1) 
        {
            close(clientFd);
            std::cerr<<"❌ fcntl() FD_CLOEXEC failed\n";
        }
        epollManager.addSocket(clientFd, EPOLLIN);
        clients.insert(std::make_pair(clientFd, Client(clientFd)));
        req.ContentLength = 0;
        clients[clientFd].statusCode = 200;
        clients[clientFd].cgi_active = 0;
        clients[clientFd].body_complete = 0;
        clients[clientFd].send_complete = 0;
        clients[clientFd].start_sending = 0;
        clients[clientFd].create_file = 0;
        clients[clientFd].Sending = 0;
        clients[clientFd].size_send = 0;
        clients[clientFd].CgiSend = 0;
        clients[clientFd].autoindex = 0;
        clients[clientFd].header_complete = 0;
        clients[clientFd].ResponseChunked = 0;
        clients[clientFd].PostBody.clear();
        clients[clientFd].CgiBody.clear();
        clients[clientFd].body_chunked.clear();
        clients[clientFd].has_cookie = 0;
        clients[clientFd].has_cgi = 0;
        clients[clientFd].cgiMap[clientFd].pipefd = -1;
        clients[clientFd].cgiMap[clientFd].pid = 0;
        clients[clientFd].cgiMap[clientFd].flag_rep = false;
        clients[clientFd].cgiMap[clientFd].signal = 0;
        clients[clientFd].cgiMap[clientFd].exit_code_cgi = 0;
        clients[clientFd].cgiMap[clientFd].Timeout = 0;
        clients[clientFd].HeaderEnd  = 0;
        clients[clientFd].chnked =0;
     //   clients[clientFd].cgiMap[clientFd].start = time(NULL);
        clients[clientFd].ContentLength = 0;
        clients[clientFd].CgiStartActivity = time(NULL);
        clients[clientFd].Read = 0;
        clients[clientFd].conf_i = 0;
        clients[clientFd].method = "";
        clients[clientFd].path.clear();
        clients[clientFd].version.clear();
        clients[clientFd].autoIndexBody.clear();
        std::cout << "✅ New client connected on fd : " << clientFd << std::endl; 
    
}

bool Server::isServerSocket(int fd) const 
{ 
    for (size_t i = 0; i < serverSockets.size(); ++i) 
    {
        if (serverSockets[i] == fd) 
            return true;
    }
    return false;
}

void Server::checkTimeout(std::map<int, Client> &clients, EpollManager &epoll, std::vector<ServerConfig> _configs)
{
    std::map<int, Client>::iterator it = clients.begin();
    while (it != clients.end())
    {
        if (difftime(time(NULL),it->second.getLastActivity()) > 5 && !clients[it->first].timeout && clients[it->first].cgiMap[it->first].pipefd == -1)
        {
            std::cerr << "⏱️ Client timed out: " << it->first << std::endl;
             if (clients[it->first].method == "GET" && !clients[it->first].ResponseChunked && !clients[it->first].send_complete && clients[it->first].Sending)
            {
                close(clients[it->first]._fd);
                std::cout << "🎉baam and Closed fd: " << clients[it->first]._fd << std::endl;
            }
            clients[it->first].response = Response::buildResponse(408, "Request Timeout", _configs[clients[it->first]. conf_i].ErrorPages[408], it->first, clients ,_configs);
            clients[it->first].timeout = true;
            epoll.modSocket(it->first, EPOLLOUT);
        }
        else
            ++it;
    }
}

void WaitChildAndClean(EpollManager &epollManager, std::map<int, Client>& clientobj, int fd, std::vector<ServerConfig> _configs)
{
     int pid = clientobj[fd].cgiMap[fd].pid;
     int wstatus = 0;
     int pipefd = -1;
    pid_t result = waitpid(pid, &wstatus, WNOHANG);
    if (result == 0) 
    {
        clientobj[fd].Read = 1;
        double elapsed = difftime(time(NULL), clientobj[fd].cgiMap[fd].start);
        if (elapsed > 3) 
        {
            clientobj[fd].cgiMap[fd].Timeout = true;
            std::cerr << "[CGI] Timeout exceeded, killing process " << pid << std::endl;
            std::cout << "[CGI] Timeout (pid=" <<clientobj[fd].cgiMap[fd].pid << ", fd=" <<clientobj[fd].cgiMap[fd].pipefd << ")\n";

            clientobj[fd].response = Response::buildResponse(504, "Gateway Timeout", _configs[clientobj[fd]. conf_i].ErrorPages[504], fd, clientobj ,_configs);
            kill(pid, SIGKILL);
            waitpid(pid, &wstatus, 0);
            pipefd = clientobj[fd].cgiMap[fd].pipefd;
            if (pipefd != -1)
            {
                std::cout << "Removing CGI pipe fd" << clientobj[fd].cgiMap[fd].pipefd << " from epoll\n";
                epoll_ctl(epollManager.getEpollFd(), EPOLL_CTL_DEL, clientobj[fd].cgiMap[fd].pipefd, NULL) ;
                close(clientobj[fd].cgiMap[fd].pipefd);
                clientobj[fd].cgiMap[fd].pipefd = -1;
            }
        }
        return;
    }               

    if (result == pid) 
    {
        if (WIFEXITED(wstatus)) 
        {
            int exitCode = WEXITSTATUS(wstatus);
                clientobj[fd].cgiMap[fd].exit_code_cgi = exitCode;

            if (exitCode == 0)
            {
                std::cout << "CGI terminé avec succès ✅\n"; 
            kill(pid, SIGKILL);
            waitpid(pid, &wstatus, 0);
            pipefd = clientobj[fd].cgiMap[fd].pipefd;
            if (pipefd != -1)
            {
                std::cout << "Removing CGI pipe fd " << pipefd << " from epoll\n";
                epoll_ctl(epollManager.getEpollFd(), EPOLL_CTL_DEL, clientobj[fd].cgiMap[fd].pipefd, NULL) ;
                close(clientobj[fd].cgiMap[fd].pipefd);
                clientobj[fd].cgiMap[fd].pipefd = -1;
            }
            }
            else
            {
                std::cerr << "CGI exited with error code " << exitCode << std::endl;
                clientobj[fd].cgiMap[fd].flag_rep = true;
                std::cout<< " flag111 == " <<clientobj[fd].cgiMap[fd].flag_rep<< std::endl;
                clientobj[fd].response = Response::buildResponse(502, "Bad Gateway", _configs[clientobj[fd]. conf_i].ErrorPages[502], fd, clientobj ,_configs);
                kill(pid, SIGKILL);
                waitpid(pid, &wstatus, 0);
                pipefd = clientobj[fd].cgiMap[fd].pipefd;
            if (pipefd != -1)
            {
                std::cout << "Removing CGI pipe fd " << pipefd << " from epoll\n";
                epoll_ctl(epollManager.getEpollFd(), EPOLL_CTL_DEL, clientobj[fd].cgiMap[fd].pipefd, NULL);
                close(clientobj[fd].cgiMap[fd].pipefd);
                clientobj[fd].cgiMap[fd].pipefd = -1;
            }
            }
        } 
        else if (WIFSIGNALED(wstatus))
         {
            clientobj[fd].cgiMap[fd].signal = true;
            clientobj[fd].response = Response::buildResponse(502, "Bad Gateway", _configs[clientobj[fd]. conf_i].ErrorPages[502], fd, clientobj ,_configs);
            std::cerr << "CGI killed by signal " << WTERMSIG(wstatus)  << "   pid    "<<clientobj[fd].cgiMap[fd].pid << std::endl;
            kill(pid, SIGKILL);
            waitpid(pid, &wstatus, 0);
            pipefd = clientobj[fd].cgiMap[fd].pipefd;
            if (pipefd != -1)
            {
                std::cout << "Removing CGI pipe fd" << pipefd << " from epoll\n";
                epoll_ctl(epollManager.getEpollFd(), EPOLL_CTL_DEL, clientobj[fd].cgiMap[fd].pipefd, NULL);
                close(clientobj[fd].cgiMap[fd].pipefd);
                clientobj[fd].cgiMap[fd].pipefd = -1;
            }
        }
    }
    pipefd = clientobj[fd].cgiMap[fd].pipefd;
    if (pipefd != -1)
    {
        std::cout << "Removing CGI pipe fd " << pipefd << " from epoll\n";
        epoll_ctl(epollManager.getEpollFd(), EPOLL_CTL_DEL, clientobj[fd].cgiMap[fd].pipefd, NULL);
        close(clientobj[fd].cgiMap[fd].pipefd);
        clientobj[fd].cgiMap[fd].pipefd = -1;
    }
}
 void init_cgi_map (std::map<int, Client>& clients, int fd)
 {
        clients[fd].cgiMap[fd].Timeout = 0;
        clients[fd].cgiMap[fd].signal = 0;
        clients[fd].cgiMap[fd].flag_rep = 0;
       // clients[fd].cgiMap[fd].start = time(NULL);
        clients[fd].cgiMap[fd].exit_code_cgi = 0;
 }

void Server::run()
{
   std::signal(SIGINT, handle_sigint);
	EpollManager epollManager;
    request a;
	for (size_t i =0; i < serverSockets.size(); i++)
	{
        epollManager.addSocket(serverSockets[i], EPOLLIN);
    }
	while (running) 
	{
		std::vector<epoll_event> events = epollManager.waitEvents();
        checkTimeout(clients, epollManager, _configs); 
        for(size_t i = 0; i < events.size(); i++)
        {
            int fd = events[i].data.fd;
            if(events[i].events & (EPOLLERR | EPOLLHUP))
            {
                if (clients[fd].cgiMap[fd].pipefd > 0 && fd != clients[fd].cgiMap[fd].pipefd) 
                {
                    std::cerr << "❌ EPOLLERR or EPOLLHUP on fd: " << fd << ", closing connection" << std::endl;
                    closeConnection(fd, epollManager);
                    continue;
                }
            }
            if (isServerSocket(fd) && (events[i].events & EPOLLIN))
                acceptNewClient(a, fd, epollManager);
            else if (clients[fd].cgiMap[fd].pipefd > 0) 
            {
               // if (!clients[fd].CgiSend)
                    init_cgi_map(clients, fd);
       // info.pid = -1;
                if (clients[fd].Read)
                    WaitChildAndClean(epollManager, clients, fd, _configs);
                else
                {
                    const size_t BUF_SIZE = 4096;
                    char buffer[BUF_SIZE];
                    ssize_t bytesRead;
                    bytesRead = read(clients[fd].cgiMap[fd].pipefd, buffer, BUF_SIZE - 1);
                    clients[fd].ResponseChunked = 0;

                    if (bytesRead > 0)
                    {
                        clients[fd].CgiSend = 1;
                        buffer[bytesRead] = '\0';
                        if (clients[fd].method == "GET")
                        {
                            clients[fd].CgiBody.append(buffer, bytesRead);
                            // clients[fd].Read = 1;
                        }
                        else if (clients[fd].method == "POST")
                        {
                            clients[fd].CGIPostBody.append(buffer, bytesRead);
                        }
                        clients[fd].bytesRead = bytesRead;
                    }
                    else if (bytesRead == 0 && clients[fd].statusCode != 204)//add timeout
                    {
                        // clients[fd].CgiBody.append(buffer, bytesRead);
                        clients[fd].statusCode = 200;
                        clients[fd].statusMsg = "OK";
                        size_t HeaderEnd = clients[fd].CgiBody.find("\r\n\r\n");
                        size_t sepLength = 4; // default CRLF

                        if (HeaderEnd == std::string::npos) {
                            HeaderEnd = clients[fd].CgiBody.find("\n\n");
                            sepLength = 2; // LF only
                        }
                        if (HeaderEnd != std::string::npos)
                        {
                            std::string headers = clients[fd].CgiBody.substr(0, HeaderEnd);
                            clients[fd].ContentType = ft_content_type(headers);
                            clients[fd].statusCode = ft_code_status(headers);
                            clients[fd].CgiBody = clients[fd].CgiBody.substr(HeaderEnd + sepLength);
                            if (clients[fd].CgiBody.empty())
                            {
                                clients[fd].response = Response::buildResponse(204, "No Content",_configs[this->clients[fd]. conf_i].ErrorPages[204], fd, clients ,_configs);
                                clients[fd].statusCode = 204;
                                clients[fd].statusMsg = "No Content";
                            }
                        }
                        WaitChildAndClean(epollManager, clients, fd, _configs);
                        std::cout << "finish reading cgi\n";
                    }
                    else if (bytesRead < 0)
                    {
                    //    if (errno == EAGAIN)
                    //    {
                    //         clients[fd].no_data = 1;
                    //    }
                    //     else
                    //     {
                    //         WaitChildAndClean(epollManager, clients, fd, _configs);
                    //         clients[fd].send_complete = 1;
                            std::cerr << "❌ read failed\n";
                    //     }
                    }
                    time_t now = time(NULL);
                    if (difftime(now, clients[fd].CgiStartActivity) > 3)
                    {
                        std::cout << "CGI timeout\n";
                        WaitChildAndClean(epollManager, clients, fd, _configs);
                        // closePipeAndCleanup(fd);
                    }
                }
                // clients[fd].has_cgi = 0;
                // clients[fd].cgiMap[fd].pipefd
            }
            else
            {
                clients[fd].no_data = 0;
                if (events[i].events & EPOLLIN) 
                {
                    try
                    {
                        if (clients[fd].send_complete == 0)
                            a = a.parseRequest(this->clients, epollManager, a, fd, _configs);
                        if (this->clients[fd].body_complete == 1 || this->clients[fd].method == "GET" ||(this->clients[fd].method.empty() && this->clients[fd].header_complete))
                        {
                            std::cout << "the requested path is: " << clients[fd].path << std::endl;
                            events[i].events = EPOLLOUT;
                            events[i].data.fd = fd;
                            if (epoll_ctl(epollManager.getEpollFd(), EPOLL_CTL_MOD, fd, &events[i]) < 0)
                                std::cerr<< "❌ epoll_ctl: mod failed ";
                        }
                        if (this->clients[fd].header_complete)
                        {
                            for (size_t i = 0; i < this->_configs.size(); ++i)
                            {
                                if (!clients[fd].get_final_port() || clients[fd].get_final_ip().empty())
                                    clients[fd].response = Response::buildResponse(400, "Bad Request", _configs[this->clients[fd]. conf_i].ErrorPages[400], fd, clients ,_configs);
                                if (this->_configs[i].port == clients[fd].get_final_port() && this->_configs[i].host == clients[fd].get_final_ip())
                                {
                                    this->clients[fd].conf_i = i; 
                                    break;
                                }
                            }
                       }
                        
                        if (this->clients[fd].body_complete == 1 || this->clients[fd].method == "GET")
                        {
                            if (!a.error_set(this->clients, fd, this->_configs[this->clients[fd].conf_i], _configs))
                            {
                                std::cout << "error_._set is: equal to zero\n";
                                throw socketException("❌ error detected  in error set");
                            }
                            else
                                handleRequest(fd, a, clients, epollManager);
                        }
                    }
                    catch(std::exception &e)
                    {
                        std::cerr << e.what() << std::endl;
                        clients[fd].response.RequestResponse(fd, clients[fd].response, clients);
                    }
                }
                else if (events[i].events & EPOLLOUT)
                {
                    std::map<int, Client>::iterator it = clients.find(fd);
                    if (it != clients.end())
                        it->second.updateActivity();
                    if ((clients[fd].method == "GET" && !clients[fd].ResponseChunked && !clients[fd].has_cgi) || (clients[fd].method == "POST" && clients[fd].has_cgi))
                        handleRequest(fd, a, clients, epollManager);
                    if (!clients[fd].no_data || clients[fd].cgiMap[fd].Timeout || clients[fd].timeout)
                        clients[fd].response.RequestResponse(fd, clients[fd].response, clients);
                    if ((clients[fd].method == "GET" && clients[fd].send_complete == 1) || clients[fd].method != "GET"
                    || (clients[fd].method == "GET" && clients[fd].ResponseChunked == 1) || clients[fd].autoindex == 1 || clients[fd].timeout)
                        closeConnection(fd, epollManager);
                }
            }
        }
    }
}
void Server::closeConnection(int fd, EpollManager &epollManager)
{
    std::map<int, Client>::iterator it = clients.find(fd);
    if (it == clients.end())
        return;
    if (it->second.cgiMap.count(fd) && it->second.cgiMap[fd].pipefd != -1)
    {
        int pipefd = it->second.cgiMap[fd].pipefd;
        std::cout << "🔸 Closing CGI pipefd: " << pipefd << std::endl;
        if (epoll_ctl(epollManager.getEpollFd(), EPOLL_CTL_DEL, pipefd, NULL) == -1)
            std::cerr << "❌ epoll_ctl DEL fd failed\n";   
        close(pipefd);
        it->second.cgiMap[fd].pipefd = -1;
    }
    std::cout << "✅ client: " << fd << " is disconnected\n";
    if (epoll_ctl(epollManager.getEpollFd(), EPOLL_CTL_DEL, fd, NULL) == -1)
        std::cerr << "❌ epoll_ctl DEL fd failed\n";
    close(fd);
    clients.erase(it);
}

Server ::socketException::socketException(const std::string &msg) :_msg(msg){}

Server ::socketException::~socketException() throw() {}


const char* Server::socketException::what() const throw()
{
    return (this->_msg).c_str();
}