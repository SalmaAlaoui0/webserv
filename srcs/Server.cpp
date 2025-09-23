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
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define WHITE   "\033[37m"
// int Server::creatServerSocket(const std::string &ip, int port)
// {
//     int server_fd  = socket(AF_INET, SOCK_STREAM, 0);  //AF_INET-->ipv4 , SOCK_STREAM-->tcp  
//     int flags = fcntl(server_fd, F_GETFL, 0);
//     if (fcntl(server_fd, F_SETFL, flags | O_NONBLOCK) < 0)
//         throw socketException("❌ Failed to set non-blocking mode on socket");
//     if (server_fd  < 0) 
//         throw socketException("❌Socket creation failed");
//     int option = 1;
//     if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)) < 0)
//     {
//         close(server_fd );
//         throw socketException("❌ Socket setup failed in setsockopt()");
//     }
//     sockaddr_in addr;
//     std::memset(&addr, 0,sizeof(addr));
//     addr.sin_family = AF_INET;
//     addr.sin_port = htons(port);
//     if (inet_pton(AF_INET, ip.c_str(), &addr.sin_addr) <= 0) 
//     {
//         close(server_fd );
//         throw socketException("❌ Invalid IP address");
//     }
//     if(bind (server_fd, (sockaddr*)&addr, sizeof(addr)) < 0)
//     {
//         close(server_fd );
//         perror("bind() failed : ");
//         throw socketException("❌ bind() failed"); 
//     }
//     if(listen(server_fd , SOMAXCONN) < 0)
//     {
//         close(server_fd );
//         throw socketException("❌ listen() failed");    
//     }
//     std::cout << "Listening on " << ip << ":" << port << std::endl;
//     return server_fd ; 
// }

int Server::creatServerSocket(const std::string &ip, int port)
{
    // 1. Create socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
        throw socketException("❌ Socket creation failed");
    // 2. Make socket non-blocking
    int flags = fcntl(server_fd, F_GETFL, 0);
    if (fcntl(server_fd, F_SETFL, flags | O_NONBLOCK) < 0) 
    {
        close(server_fd);
        throw socketException("❌ Failed to set non-blocking mode on socket");
    }
    // 3. Allow address reuse
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) 
    {
        close(server_fd);
        throw socketException("❌ setsockopt(SO_REUSEADDR) failed");
    }
    // 4. Prepare hints
    struct addrinfo hints;
    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_INET;       // IPv4
    hints.ai_socktype = SOCK_STREAM;   // TCP
    hints.ai_flags    = AI_PASSIVE;    // For binding
    // Convert port to string
    std::stringstream ss;
    ss << port;
    std::string portStr = ss.str();
    // 5. Resolve address
    struct addrinfo *res;
    const char *host = ip.empty() ? NULL : ip.c_str();
    int status = getaddrinfo(host, portStr.c_str(), &hints, &res);
    if (status != 0) 
    {
        close(server_fd);
        throw socketException(std::string("❌ getaddrinfo: ") + gai_strerror(status));
    }
    // 6. Bind socket
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

    std::cout << GREEN << "✅ Listening on "<< CYAN << (ip.empty() ? "0.0.0.0" : ip)
          << RESET << ":"<< YELLOW << port<< RESET << std::endl;
          
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

Server::Server(const std::vector<ServerConfig>& configs)
{
    this->_configs = configs;
    setupSockets();
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
        clients[clientFd].response = Response::buildResponse(r, 500, "Internal Server Error (no matching server)",_configs[this->clients[clientFd]. conf_i].ErrorPages[500], clientFd, clients);
        return;
    }
    if (r.get_path() == "/favicon.ico")
    {
        clients[clientFd].response = Response::buildResponse(r, 404, "Not Found",_configs[this->clients[clientFd]. conf_i].ErrorPages[404], clientFd, clients);
        return;
    }
	if (r.get_method() == "GET")
		handle_get_methode(r, this->_configs, clientFd, this->clients[clientFd]. conf_i, clientobj, epoll);
    else if(clientobj[clientFd].method== "POST")
        handle_post_methode(r, this->_configs, clientFd, this->clients[clientFd]. conf_i, clientobj, epoll);
    else if (r.get_method() == "DELETE")
		handle_delete_methode(r, this->_configs, clientFd, this->clients[clientFd]. conf_i, clientobj);
    else
    {
        clients[clientFd].response = Response::buildResponse(r, 404, "invalid method",_configs[this->clients[clientFd]. conf_i].ErrorPages[404], clientFd, clients);
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
        std::cout << "[DEBUG] accepted new client got fd " << clientFd << "\n";
                // std::cout << "client fd" << clientFd << std::endl;
        int flags = fcntl(clientFd, F_GETFL, 0);
        if (fcntl(clientFd, F_SETFL, flags | O_NONBLOCK) == -1 )
        {
            close(clientFd);
            throw socketException("❌fcntl() failed");
        }
        if (clientFd < 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                return;
            throw socketException("❌ accept failed");
        }
        epollManager.addSocket(clientFd, EPOLLIN);
        clients.insert(std::make_pair(clientFd, Client(clientFd)));
        req.ContentLength = 0;
        clients[clientFd].cgi_active = 0;
        clients[clientFd].body_complete = 0;
        clients[clientFd].send_complete = 0;
        clients[clientFd].start_sending = 0;
        clients[clientFd].create_file = 0;
        clients[clientFd].Sending = 0;
        clients[clientFd].autoindex = 0;
        clients[clientFd].header_complete = 0;
        clients[clientFd].ResponseChunked = 0;
        clients[clientFd].PostBody.clear();
        clients[clientFd].CgiBody.clear();
        clients[clientFd].has_cookie = 0;
        clients[clientFd].has_cgi = 0;
        clients[clientFd].cgiMap[clientFd].pipefd = -1;
        clients[clientFd].ContentLength = 0;
        clients[clientFd].CgiStartActivity = time(NULL);
        std::cout << "\n✅ New client connected on fd : " << clientFd << std::endl;
    
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

void Server::checkTimeout(std::map<int, Client> &clients, EpollManager &epoll)
{
    time_t now = std::time(NULL);
    std::map<int, Client>::iterator it = clients.begin();
    while (it != clients.end())
    {
        now = it->second.getLastActivity();
        if (now - it->second.getLastActivity() > 60)
        {
            std::cout << "⏱️ Client timed out: " << it->first << std::endl;
            epoll_ctl(epoll.getEpollFd(), EPOLL_CTL_DEL, it->first, NULL);
            close(it->first);
            std::map<int, Client>::iterator tmp = it;
            ++it;
            clients.erase(tmp);
        }
        else
            ++it;
    }
}


void WaitChildAndClean(EpollManager &epollManager, std::map<int, Client>& clientobj, int fd)
{
    int pid = clientobj[fd].cgiMap[fd].pid;
    int wstatus;
    pid_t result = waitpid(pid, &wstatus, WNOHANG);
    if (result == 0)
    {
        kill(pid, SIGKILL);
        waitpid(pid, &wstatus, 0);
    }
    int pipefd = clientobj[fd].cgiMap[fd].pipefd;
    if (pipefd != -1)
    {
        std::cout << "Removing CGI pipe fd " << pipefd << " from epoll\n";
        epoll_ctl(epollManager.getEpollFd(), EPOLL_CTL_DEL, clientobj[fd].cgiMap[fd].pipefd, NULL);
        close(clientobj[fd].cgiMap[fd].pipefd);
        clientobj[fd].cgiMap[fd].pipefd = -1;
        std::cout << "\n\n->cgibody: " << clientobj[fd].CgiBody << "<-\n\n";
        // exit (2);
    }
    ;
}

void Server::run()
{
	EpollManager epollManager;
    request a;
    Server s;
	for (size_t i =0; i < serverSockets.size(); i++)
	{
        epollManager.addSocket(serverSockets[i], EPOLLIN);
        std::cout << "new socket added to lesten for any upcoming connections" << std::endl;
    }
	while (true) 
	{
		std::vector<epoll_event> events = epollManager.waitEvents();
        checkTimeout(clients, epollManager);
        for(size_t i = 0; i < events.size(); i++)
        {   
            int fd = events[i].data.fd;
            if(events[i].events & (EPOLLERR | EPOLLHUP))
            {
                closeConnection(fd, epollManager);
                continue;
            }
            if (isServerSocket(fd) && (events[i].events & EPOLLIN))
                acceptNewClient(a, fd, epollManager);
            else if (clients[fd].cgiMap[fd].pipefd != -1 && clients[fd].cgiMap[fd].pipefd != 0) {
                const size_t BUF_SIZE = 4096;
                char buffer[BUF_SIZE];
                ssize_t bytesRead;
                bytesRead = read(clients[fd].cgiMap[fd].pipefd, buffer, BUF_SIZE);
                clients[fd].ResponseChunked = 0;
                if (bytesRead > 0)
                {
                    std::cout << "The Body Is Being Appended. " << std::endl;
                    clients[fd].CgiBody.append(buffer, bytesRead);
                    // std::cout << "\n\n************->cgiappended buffer: " << buffer << "<-\n\n";
                    // clients[fd].ResponseChunked = 0;
                }
                else if (bytesRead == 0)//add timeout
                {
                    std::cout << "bytesRead is : 0" << std::endl;
                    clients[fd].CgiBody.append(buffer, bytesRead);
                    clients[fd].statusCode = 200;
                    clients[fd].statusMsg = "OK";
                    WaitChildAndClean(epollManager, clients, fd);
                    std::cout << "finish reading cgi\n";
                    // clients[fd].send_complete = 1;
                }
                else if (bytesRead < 0)
                {
                    if (errno == EAGAIN)
                        clients[fd].no_data = 1;
                    else
                    {
                        WaitChildAndClean(epollManager, clients, fd);
                        clients[fd].send_complete = 1; // this should become send complete because 
                        // I should make an appropriate html response
                        perror("REad");
                        // exit (23);
                    }
                }
                // if 
                time_t now = time(NULL);
                if (difftime(now, clients[fd].CgiStartActivity) > 3)
                {
                    std::cout << "CGI timeout\n";
                    WaitChildAndClean(epollManager, clients, fd);
                    // closePipeAndCleanup(fd);
                }
                // clients[fd].has_cgi = 0;
                // clients[fd].cgiMap[fd].pipefd
            }
            else
            {
                clients[fd].no_data = 0;
                if (events[i].events & EPOLLIN) 
                {
                    // std::cout << "Socket ❌❌❌❌❌ " << fd << " is ready to read\n";
                    try
                    {
                        if (clients[fd].send_complete == 0)
                            a = a.parseRequest(this->clients, epollManager, a, fd);
                        if (this->clients[fd].body_complete == 1 || this->clients[fd].method == "GET")
                        {
                            std::cout << "\nMaking the event EPOLLOUT \n";
                            events[i].events = EPOLLOUT;
                            events[i].data.fd = fd;
                            if (epoll_ctl(epollManager.getEpollFd(), EPOLL_CTL_MOD, fd, &events[i]) == -1) {
                                perror("epoll_ctl: mod");
                            }
                        }
                        this->clients[fd].conf_i = s.getConfig().size();
                        for (size_t i = 0; i < s.getConfig().size(); ++i)
                        {
                            if (s.getConfig()[i].port == a.get_final_port(a))
                            { 
                                this->clients[fd].conf_i = i; 
                                break;
                            }
                        }
                        if (this->clients[fd].body_complete == 1 || this->clients[fd].method == "GET")
                        {
                            if (!a.error_set(this->clients, a, fd, s.getConfig()[this->clients[fd].conf_i]))
                                throw socketException("❌ error detected ");
                            else
                                handleRequest(fd, a, clients, epollManager);
                        }
                        std::map<int, Client>::iterator it = clients.find(fd);
                        if (it != clients.end())
                            it->second.updateActivity();
                    }
                    catch(std::exception &e)
                    {
                        std::cout << e.what() << std::endl;
                    }
                }
                else if (events[i].events & EPOLLOUT)
                {
                    if (clients[fd].method == "GET" && !clients[fd].ResponseChunked && !clients[fd].has_cgi)
                        handleRequest(fd, a, clients, epollManager);
                    if (!clients[fd].no_data)
                    {
                        clients[fd].response.RequestResponse(fd, clients[fd].response, clients, epollManager);
                    }
                    if ((clients[fd].method == "GET" && clients[fd].send_complete == 1) || clients[fd].method != "GET"
                     || (clients[fd].method == "GET" && clients[fd].ResponseChunked == 1) || clients[fd].autoindex == 1)
                    {
                        closeConnection(fd, epollManager);
                        // close(fd);
                        // std::cout << "✅ client: " << fd << " is disconnected\n";
                    }
                }
                else
                    std::cout<<"maart ach kandir hna\n\n";
            }
        }
    }
}


void Server::closeConnection(int fd, EpollManager &epollManager)
{
    if (clients.count(fd))
    {
        int pipefd = clients[fd].cgiMap[fd].pipefd;
        if (pipefd > 2)
        {
            std::cout << "Closing CGI pipefd: " << pipefd << "\n";
            if (epoll_ctl(epollManager.getEpollFd(), EPOLL_CTL_DEL, pipefd, NULL) == -1)
                    perror("epoll_ctl DEL pipefd");
            close(pipefd);
            clients[fd].cgiMap[fd].pipefd = -1;
        }
    }
    if (epoll_ctl(epollManager.getEpollFd(), EPOLL_CTL_DEL, fd, NULL) == -1)
        perror("epoll_ctl DEL fd");
    close(fd);
    clients.erase(fd);
    std::cout << "✅ client: " << fd << " is disconnected\n";
    // std::cout << "🚪 Closed client fd: " << fd << std::endl;
}

Server ::socketException::socketException(const std::string &msg) :_msg(msg){}

Server ::socketException::~socketException() throw() {}


const char* Server::socketException::what() const throw()
{
    return (this->_msg).c_str();
}