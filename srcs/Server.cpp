/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: wzahir <wzahir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/08 15:25:50 by wzahir            #+#    #+#             */
/*   Updated: 2025/08/29 14:34:29 by wzahir           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */



#include "../includes/Server.hpp"

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

int Server::creatServerSocket(const std::string &ip, int port)
{
    int server_fd  = socket(AF_INET, SOCK_STREAM, 0);
    if (fcntl(server_fd, F_SETFL, O_NONBLOCK) < 0)
        throw socketException("❌ Failed to set non-blocking mode on socket");
    if (server_fd  < 0) 
        throw socketException("❌Socket creation failed");
    int option = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)) < 0)
    {
        close(server_fd );
        throw socketException("❌ Socket setup failed in setsockopt()");
    }
    sockaddr_in addr;
    std::memset(&addr, 0,sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip.c_str(), &addr.sin_addr) <= 0) 
    {
        close(server_fd );
        throw socketException("❌ Invalid IP address");
    }
    if(bind (server_fd, (sockaddr*)&addr, sizeof(addr)) < 0)
    {
        close(server_fd );
        perror("bind() failed : ");
        throw socketException("❌ bind() failed"); 
    }
    if(listen(server_fd , SOMAXCONN) < 0)
    {
        close(server_fd );
        throw socketException("❌ listen() failed");    
    }
    std::cout << "Listening on " << ip << ":" << port << std::endl;
    return server_fd ; 
}

void Server::sendResponse( int clientFd, request &r)
{

	if (r.get_path() == "/favicon.ico")
	{
		std::string notFound = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
		if (send(clientFd, notFound.c_str(), notFound.size(), 0) < 0)
            std::cerr << "❌ send failed: " << strerror(errno) << std::endl; 
		return;
	}
    size_t conf_i = _configs.size();
    for (size_t i = 0; i < _configs.size(); ++i) 
    {
        if (_configs[i].port == r.get_final_port(r))
        { 
            conf_i = i; 
            break;
        }
    }
    if (conf_i == _configs.size()) 
    {
		send_response(clientFd, 500, "Internal Server Error (no matching server)", load_html_file("www/500.html"));
        return;
    }
	if (r.get_method() == "GET")
		handle_get_methode(r, this->_configs, clientFd, conf_i);
    else if(r.get_method()== "POST")
        handle_post_methode(r, this->_configs, clientFd, conf_i);
    else if (r.get_method() == "DELETE")
		handle_delete_methode(r, this->_configs, clientFd, conf_i);
    else
    {
        std::string invalidMethod = "HTTP/1.1 404 invalid method\r\nContent-Length: 0\r\n\r\n";
		if (send(clientFd, invalidMethod.c_str(), invalidMethod.size(), 0) < 0)
            std::cerr << "❌ send failed: " << strerror(errno) << std::endl;    
		return;
    }
    /// send output
    //return;
}

// void Server::handleClient(int clientFd, EpollManager &epollManager)
// {
//     request a;
//     try
//     {
//         a = a.parseRequest(this->clients, epollManager, a, clientFd);
//     }
//     catch(std::exception &e)
//     {
//         std::cout << e.what() << std::endl;
//         return ;
//     }
//     std::cout << "fd = "<< clientFd << std::endl;
//     if (a.error_set(this->clients, a, clientFd) == 1)
//     {
//         if (clients[clientFd].body_complete == 1)
//             sendResponse(clientFd, a);
//     }
//     // std::map<std::string, std::string> response = e.what();
//     // send_response(clientFd, 400, "Bad Request", load_html_file("www/403.html"));
//     // // ssize_t sent = send(clientFd, response.c_str(), response.size(), 0);
//     // // if (sent < 0)
//     // //     std::cerr << "❌ send failed: " << strerror(errno) << std::endl;
//     // // else
//     // //     std::cout << "Response sent to FD: " << clientFd << std::endl;
//     // return ;
//     std::cout << "\n\n\nHEREEEEEEEEEEEEEEEEEEEEEEEE"  << "---------------\n\n\n";
// 	std::map<int, Client>::iterator it = clients.find(clientFd);
// 	if (it != clients.end())
// 		it->second.updateActivity();
//     // if (clients[clientFd].body_complete == 1)
//     // {
//     //     close(clientFd);
//     //     std::cout << "client has been closed after sending response :))\n";
//     // }
// }
// >>>>>>> 71c16ac229040537c0606ef107d6f7e8cf5d73b8

void Server::acceptNewClient(int serverFd, EpollManager &epollManager)
{
        struct sockaddr clientAddr;
        socklen_t clientLen = sizeof(clientAddr);
        int clientFd = accept(serverFd, (sockaddr *)&clientAddr , &clientLen);
                std::cout << "client fd" << clientFd << std::endl;

        if (fcntl(clientFd, F_SETFL, O_NONBLOCK) == -1)
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
        std::cout << "error not here\n";
        // epollManager.addSocket(clientFd);
        clients.insert(std::make_pair(clientFd, Client(clientFd)));
        clients[clientFd].body_complete = 0;
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

void Server::run()
{
	EpollManager epollManager;
	for (size_t i =0; i < serverSockets.size(); i++)
	{
        epollManager.addSocket(serverSockets[i]);
        std::cout << "new socket added to lesten for any upcoming connections" << std::endl;
    }
	while (true) 
	{
		std::vector<epoll_event> events = epollManager.waitEvents();
        checkTimeout(clients, epollManager);
        for(size_t i = 0; i < events.size(); i++)
        {   
            int fd = events[i].data.fd;
            // if (events[i].events & EPOLLIN) 
            // {
            //     std::cout << "Socket " << fd << " is ready to read\n";
            // }
            // if (events[i].events & EPOLLOUT) 
            // {
            //     std::cout << "Socket " << fd << " is ready to write\n";
            // }
            if (isServerSocket(fd))
                acceptNewClient(fd, epollManager);
            else
            {
                //handleClient(fd, epollManager, events);
                request a;
                if (events[i].events & EPOLLIN) 
                {
                    std::cout << "Socket ❌❌❌❌❌ " << fd << " is ready to read\n";
                    try
                    {
                        a = a.parseRequest(this->clients, epollManager, a);
                    }
                    catch(std::exception &e)
                    {
                        std::cout << e.what() << std::endl;
                        return ;
                    }
                    std::cout << "fd = "<< fd << std::endl;
                   
                        return;
                }
                if (events[i].events & EPOLLOUT)
                {
                    std::cout << "Socket ❌❌❌❌❌" << fd << " is ready to write\n";
                     if (a.error_set(a, fd) == 1)
                     {
                         sendResponse(fd, a);  
                     }
                        // std::map<std::string, std::string> response = e.what();
                        // send_response(fd, 400, "Bad Request", load_html_file("www/403.html"));
                        // // ssize_t sent = send(fd, response.c_str(), response.size(), 0);
                        // // if (sent < 0)
                        // //     std::cerr << "❌ send failed: " << strerror(errno) << std::endl;
                        // // else
                        // //     std::cout << "Response sent to FD: " << fd << std::endl;
                        // return ;
                        std::map<int, Client>::iterator it = clients.find(fd);
                        if (it != clients.end())
                            it->second.updateActivity();
                        close(fd);
                        std::cout << "client has been closed after sending response :))\n";
                }
                else
                    std::cout<<"maart ach kandir hna\n\n";
        }
    }
}

void Server::closeConnection(int fd, EpollManager &epollManager)
{
    epollManager.delSocket(fd);
    close(fd);
    clients.erase(fd);
    std::cout << "🚪 Closed client fd: " << fd << std::endl;
}

Server ::socketException::socketException(const std::string &msg) :_msg(msg){}

Server ::socketException::~socketException() throw() {}


const char* Server::socketException::what() const throw()
{
    return (this->_msg).c_str();
}
