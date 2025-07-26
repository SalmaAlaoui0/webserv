/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: salaoui <salaoui@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/08 15:25:50 by wzahir            #+#    #+#             */
/*   Updated: 2025/07/26 10:20:50 by salaoui          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Server.hpp"

Server::Server(const std::vector<ServerConfig>& configs)
{
    this->_configs = configs;
    setupSockets();
}

Server::Server()
{
}
Server::~Server()
{
    for (size_t i = 0; i < serverSockets.size(); ++i)
        close(serverSockets[i]);
}

void Server:: setupSockets()
{
    for(size_t i = 0; i < _configs.size(); i++)
    {
        if ( _configs[i].port != 0)
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

void Server::sendResponse( int clientFd, request r)
{
//     std::string body = R"(<!DOCTYPE html> <html lang="en">
// <head><h1><center>Hello world</center></h1>
// </head>
// </html>)";
	//(void)r;
    // std::string body = "<h1><center>Hello world</center></h1>";
	if (r.get_path() == "/favicon.ico")
	{
		std::string notFound = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
		send(clientFd, notFound.c_str(), notFound.size(), 0);
		return;
	}
    
	// std::cout << "your method is: " << r.get_method() << ", and the path is(ps: without file it's stored alone;): " << r.get_root() << ", and version is: " << r.get_version() << std::endl << std::endl;
	if (r.get_method() == "GET")
		handle_get_methode(r, this->_configs, clientFd);
    if(r.get_method()== "POST")
        handle_post_methode(r, this->_configs, clientFd,r.get_final_port(r));
    // std::string body = "<h1><center>Hello world</center></h1>";
    // std::ostringstream response;
	// response << "HTTP/1.1 200 OK\r\n"
	// 		 << "Content-Type: text/html\r\n"
	// 		 << "Content-Length: " << body.size() << "\r\n\r\n" << body;

    // ssize_t sent = send(clientFd, response.str().c_str(), response.str().size(), 0);
    // if (sent < 0)
    //     std::cerr << "❌ send failed: " << strerror(errno) << std::endl;
    // else
    //     std::cout << "Response sent to FD: " << clientFd << std::endl;
}


void Server::handleClient(int clientFd, EpollManager &epollManager)
{
    request a;
    try{
        a = a.parseRequest(clientFd, epollManager, a);
    }
    catch(std::exception &e)
    {
        std::cout << e.what() << std::endl;
        return ;
    }
    try{
        a.error_set(a);
    }
    catch(std::exception &e)
    {
    std::string response = e.what();
    ssize_t sent = send(clientFd, response.c_str(), response.size(), 0);
    if (sent < 0)
        std::cerr << "❌ send failed: " << strerror(errno) << std::endl;
    else
        std::cout << "Response sent to FD: " << clientFd << std::endl;
    return ;
    }
    sendResponse(clientFd, a);
	std::map<int, Client>::iterator it = clients.find(clientFd);
	if (it != clients.end())
	{
		it->second.updateActivity();
	}
}

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
        epollManager.addSocket(clientFd);
        clients.insert(std::make_pair(clientFd, Client(clientFd)));
        // std::cout << "✅ New client connected on fd : " << clientFd << std::endl;
    
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
        std::cout << "new socket added to lesten for any upcoming connections" << std::endl;
        epollManager.addSocket(serverSockets[i]);
    }
	while (true) 
	{
		std::vector<int> fds = epollManager.waitEvents(*this);
		checkTimeout(clients, epollManager);
		for (size_t i = 0; i < fds.size(); ++i) 
		{
			if (isServerSocket(fds[i])) 
			{
                // std::cout << "hehehahahahh just detected a new connection\n" << std::endl;
                acceptNewClient(fds[i], epollManager);
            }
			else 
			{
               // std::cout << "started handling an already there connection\n" << std::endl;
				handleClient(fds[i], epollManager);
				// char buf[1024];
				// int bytes = read(fds[i], buf, sizeof(buf));
				// if (bytes <= 0) {
				//     std::cout << "❌ Client disconnected " << fds[i] << std::endl;
				//     close(fds[i]);
				// } else {
				//     std::cout << "📥 Received: " << std::string(buf, bytes) << std::endl;
				//     std::string response = "HTTP/1.1 200 OK\r\nContent-Length: 12\r\n\r\nHello world";
				//     send(fds[i], response.c_str(), response.size(), 0);
				// }
            }
        }
    }
}

void Server::closeClient(int fd, EpollManager &epollManager)
{
    epoll_ctl(epollManager.getEpollFd(), EPOLL_CTL_DEL, fd, NULL);
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
