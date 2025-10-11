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
    // int flags = fcntl(server_fd, F_GETFL, 0);
    // if (fcntl(server_fd, F_SETFL, flags | O_NONBLOCK) < 0) 
    // {
    //     close(server_fd);
    //     throw socketException("❌ Failed to set non-blocking mode on socket");
    // }
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

Server::Server(const std::vector<ServerConfig> &configs)
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
    if (clientobj[clientFd].method  == "/favicon.ico")
    {
        std::cout << "hereeee: the favicon thing\n";
        clients[clientFd].response = Response::buildResponse(r, 404, "Not Found",_configs[this->clients[clientFd]. conf_i].ErrorPages[404], clientFd, clients);
        return;
    }
	if (clientobj[clientFd].method == "GET")
        handle_get_methode(r, this->_configs, clientFd, this->clients[clientFd]. conf_i, clientobj, epoll);
    else if(clientobj[clientFd].method== "POST")
    {
        std::cout << " hiiiiiiiiiiiiii222222\n\n";
        handle_post_methode(r, this->_configs, clientFd, this->clients[clientFd]. conf_i, clientobj, epoll);

    }
    else if (clientobj[clientFd].method  == "DELETE")
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
        clients[clientFd].statusCode = 200;
        clients[clientFd].cgi_active = 0;
        clients[clientFd].body_complete = 0;
        clients[clientFd].send_complete = 0;
        clients[clientFd].start_sending = 0;
        clients[clientFd].create_file = 0;
        clients[clientFd].Sending = 0;
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
        std::cout << "\n✅ New client connected on fd : " << clientFd << std::endl; 
    
}

bool Server::isServerSocket(int fd) const 
{
    // std::cout << "\n✅✅✅✅✅ New client connected on fd : " << std::endl; 
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
        if (now - it->second.getLastActivity() > 10)
        {
            std::cerr << "⏱️ Client timed out: " << it->first << std::endl;
           // clientobj[fd].response = Response::buildResponse(a, 502, "Bad Gateway", _configs[clientobj[fd]. conf_i].ErrorPages[502], fd, clientobj);
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


void WaitChildAndClean(EpollManager &epollManager, std::map<int, Client>& clientobj, int fd, std::vector<ServerConfig> _configs, request &a)
{
     int pid = clientobj[fd].cgiMap[fd].pid;
     int wstatus = 0;
    pid_t result = waitpid(pid, &wstatus, WNOHANG);
    clientobj[fd].send_complete = 1;
if (result == 0) {
    double elapsed = difftime(time(NULL), clientobj[fd].cgiMap[fd].start);
    if (elapsed > 3) {
        clientobj[fd].cgiMap[fd].Timeout = true;
        std::cerr << "[CGI] Timeout exceeded, killing process " << pid << std::endl;
        clientobj[fd].response = Response::buildResponse(a, 502, "Bad Gateway", _configs[clientobj[fd]. conf_i].ErrorPages[502], fd, clientobj);
        kill(pid, SIGKILL);
        waitpid(pid, &wstatus, 0);
         close(clientobj[fd].cgiMap[fd].pipefd);
        clientobj[fd].cgiMap[fd].pipefd = -1;
    }
    return;
}

if (result == pid) {
    if (WIFEXITED(wstatus)) {
        int exitCode = WEXITSTATUS(wstatus);
             clientobj[fd].cgiMap[fd].exit_code_cgi = exitCode;

        if (exitCode == 0)
            std::cout << "CGI terminé avec succès ✅\n"; 
        else
        {
            std::cerr << "CGI exited with error code " << exitCode << std::endl;
            clientobj[fd].cgiMap[fd].flag_rep = true;
            std::cout<< " flag111 == " <<clientobj[fd].cgiMap[fd].flag_rep<< std::endl;
			clientobj[fd].response = Response::buildResponse(a, 502, "Bad Gateway", _configs[clientobj[fd]. conf_i].ErrorPages[502], fd, clientobj);
            kill(pid, SIGKILL);
        waitpid(pid, &wstatus, 0);
         close(clientobj[fd].cgiMap[fd].pipefd);
        clientobj[fd].cgiMap[fd].pipefd = -1;
        }
    } 
    else if (WIFSIGNALED(wstatus)) {
        clientobj[fd].cgiMap[fd].signal = true;
		clientobj[fd].response = Response::buildResponse(a, 502, "Bad Gateway", _configs[clientobj[fd]. conf_i].ErrorPages[502], fd, clientobj);
         close(clientobj[fd].cgiMap[fd].pipefd);
        clientobj[fd].cgiMap[fd].pipefd = -1;
        std::cerr << "CGI killed by signal " << WTERMSIG(wstatus)  << "   pid    "<<clientobj[fd].cgiMap[fd].pid << std::endl;
        kill(pid, SIGKILL);
        waitpid(pid, &wstatus, 0);
    }
}
int pipefd = clientobj[fd].cgiMap[fd].pipefd;
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
        clients[fd].cgiMap[fd].start = 0;
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
        std::cout << "new socket added to lesten for any upcoming connections" << std::endl;
    }
	while (running) 
	{
		std::vector<epoll_event> events = epollManager.waitEvents();
        checkTimeout(clients, epollManager);
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
            else if (clients[fd].cgiMap[fd].pipefd > 0 && !clients[fd].Read) {
                init_cgi_map(clients, fd);
       // info.pid = -1;
                // std::cout << "Handling CGI pipe read for fd: " << fd << ", pipefd: " << clients[fd].cgiMap[fd].pipefd << std::endl;
                // std::cout << "\n\n Here in reading cgi pipe content\n\n";
                // exit (18);
                const size_t BUF_SIZE = 4096;
                char buffer[BUF_SIZE];
                ssize_t bytesRead;
                bytesRead = read(clients[fd].cgiMap[fd].pipefd, buffer, BUF_SIZE - 1);
                // clients[fd].ResponseChunked = 0;
              //   std::cout << "\n\n Here in reading cgi pipe content buffer:++>" << bytesRead << "<--\n\n";
                // exit (18);

                if (  bytesRead == 0 && !clients[fd].CgiSend)
                {
                    std::cout << "No data read from CGI pipe, setting CgiEmptyContent to 1 in the fd number: " << fd << "\n";
                    clients[fd].response = Response::buildResponse(a, 204, "No Content",_configs[this->clients[fd]. conf_i].ErrorPages[204], fd, clients);
                    clients[fd].statusCode = 204;
                    clients[fd].statusMsg = "No Content";
                    WaitChildAndClean(epollManager, clients, fd, _configs, a);
                    // exit(13);
                }
                if (bytesRead > 0)
                {
                    clients[fd].CgiSend = 1;
                    buffer[bytesRead] = '\0';
                    // std::cout << "The Body Is Being Appended. " << std::endl;
                    if (clients[fd].method == "GET")
                    {
                        clients[fd].CgiBody.assign(buffer, bytesRead);
                        clients[fd].Read = 1;
                        // std::cout << "hereere READINNG IS: 1\n";
                    }
                    else if (clients[fd].method == "POST")
                    {
                        clients[fd].CGIPostBody.append(buffer, bytesRead);
                     //   std::cout << "\n\n Here in reading cgi pipe content buffer:++>" << buffer << "<--\n\n";
                        // exit (19);
                    }
                    clients[fd].bytesRead = bytesRead;
                }
                else if (bytesRead == 0 && clients[fd].statusCode != 204)//add timeout
                {
                    std::cout << "bytesRead is : 0" << std::endl;
                    clients[fd].CgiBody.append(buffer, bytesRead);
                    clients[fd].statusCode = 200;
                    clients[fd].statusMsg = "OK";
                    WaitChildAndClean(epollManager, clients, fd, _configs, a);
                    std::cout << "finish reading cgi\n";
                    // clients[fd].send_complete = 1;
                }
                else if (bytesRead < 0)
                {
                    if (errno == EAGAIN)
                    {
                        clients[fd].no_data = 1;
                    }
                    else
                    {
                        WaitChildAndClean(epollManager, clients, fd, _configs, a);
                        clients[fd].send_complete = 1;
                        // make an error html response
                        perror("REad");
                        // exit (23);
                    }
                }
                time_t now = time(NULL);
                if (difftime(now, clients[fd].CgiStartActivity) > 3)
                {
                    std::cout << "CGI timeout\n";
                    WaitChildAndClean(epollManager, clients, fd, _configs, a);
                    // closePipeAndCleanup(fd);
                }
                // clients[fd].has_cgi = 0;
                // clients[fd].cgiMap[fd].pipefd
            }
            else
            {
               // std::cout<<"❌❌❌❌❌" << clients[fd].send_complete  << "  header  : "<< this->clients[fd].header_complete <<"  methode  : "<<  this->clients[fd].method<<std::endl;
                clients[fd].no_data = 0;
                if (events[i].events & EPOLLIN) 
                {
                    // std::cout << "Socket ❌❌❌❌❌ " << fd << " is ready to read\n";
                    try
                    {
                        if (clients[fd].send_complete == 0)
                        {
                            a = a.parseRequest(this->clients, epollManager, a, fd);
                            std::map<int, Client>::iterator it = clients.find(fd);
                            if (it != clients.end())
                                it->second.updateActivity();
                            std::cout << "timeeeeeeee is: " << clients[fd].getLastActivity() << "<==========\n\n0";
                        }
                        if (this->clients[fd].body_complete == 1 || this->clients[fd].method == "GET" ||(this->clients[fd].method.empty() && this->clients[fd].header_complete))
                        {
                            std::cout << "\nMaking the event EPOLLOUT \n";
                            events[i].events = EPOLLOUT;
                            events[i].data.fd = fd;
                            if (epoll_ctl(epollManager.getEpollFd(), EPOLL_CTL_MOD, fd, &events[i]) == -1) {
                                perror("epoll_ctl: mod");
                            }
                        }
                        if (this->clients[fd].header_complete)
                        {
                            for (size_t i = 0; i < this->_configs.size(); ++i)
                            {
                             //   std::cout << "the port in config is: " << this->_configs[i].port << " and the final port is: " << clients[fd].get_final_port() << std::endl;
                                if (!clients[fd].get_final_port()) // CHEKK WITH IP AND PORT AND IF LOCALHOST-->CONVERT 127.0.0.1
                                {
                                    std::cout << "BBBad request msgg and methode is: " << clients[fd].method.empty() << "\n\n";
                                    clients[fd].response = Response::buildResponse(a, 400, "Bad Request", "", fd, clients);
                                }
                                if (this->_configs[i].port == 8080)
                                { 
                                    this->clients[fd].conf_i = i; 
                                    break;
                                }
                            }
                       }
                        
                        if (this->clients[fd].body_complete == 1 || this->clients[fd].method == "GET")
                        {
                            std::cout << "hereee is\n\n";

                            if (!a.error_set(this->clients, a, fd, this->_configs[this->clients[fd].conf_i]))
                            {
                                std::cout << "error_._set is: equal to zero\n";
                                throw socketException("❌ error detected  in error set");
                            }
                            else
                            {
                                 std::cout << "______________________Hrererere is the leakkk for fd: " << fd << std::endl;

                                // std::cout << "Hrererere is the leakkk for fd: handle " << fd << std::endl;

                                handleRequest(fd, a, clients, epollManager);
                                        // Read from the pipe directly
                                // char buffer[4096];
                                // ssize_t bytesRead;
                                // std::string pipeOutput;

                                // while ((bytesRead = read(clients[fd].cgiMap[fd].pipefd, buffer, sizeof(buffer) - 1)) > 0) {
                                //     buffer[bytesRead] = '\0';
                                //     pipeOutput += buffer;
                                // }

                                // std::cout << "DEBUG: CGI output from pipe:\n" << pipeOutput << std::endl;
                            }
                        

                }
                    }
                    catch(std::exception &e)
                    {
                        std::cout << e.what() << std::endl;
                        std::cout << " heloooooo|\n\n\n\n";

                            clients[fd].response.RequestResponse(fd, clients[fd].response, clients);
                    }
                }
                else if (events[i].events & EPOLLOUT)
                    {
                        if ((clients[fd].method == "GET" && !clients[fd].ResponseChunked && !clients[fd].has_cgi) || (clients[fd].method == "POST" && clients[fd].has_cgi))
                        {
                            std::cout<<"+++++++++++++++++++maart ach kandir hna f handle req 2\n\n";

                            handleRequest(fd, a, clients, epollManager);

                        }
                        if (!clients[fd].no_data)
                            clients[fd].response.RequestResponse(fd, clients[fd].response, clients);
                        if ((clients[fd].method == "GET" && clients[fd].send_complete == 1) || clients[fd].method != "GET"
                        || (clients[fd].method == "GET" && clients[fd].ResponseChunked == 1) || clients[fd].autoindex == 1)
                        {
                            std::cout << " closeeeeeeeeeeeeeeeee\n\n";
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
  // clients[fd].~Client();
    std::cout << "✅ client: " << fd << " is disconnected\n";
    // std::cout << "🚪 Closed client fd: " << fd << std::endl;
}

Server ::socketException::socketException(const std::string &msg) :_msg(msg){}

Server ::socketException::~socketException() throw() {}


const char* Server::socketException::what() const throw()
{
    return (this->_msg).c_str();
}