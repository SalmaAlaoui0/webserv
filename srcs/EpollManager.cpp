/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EpollManager.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: salaoui <salaoui@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/13 17:11:36 by wzahir            #+#    #+#             */
/*   Updated: 2025/07/20 09:43:34 by salaoui          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/EpollManager.hpp"

EpollManager::EpollManager()
{
    epollFd = epoll_create(1024);
    if (epollFd == -1)
        throw epollException("❌ Failed to create epoll instance");
}

EpollManager::~EpollManager()
{
    close(epollFd);
}

void EpollManager::addSocket(int fd)
{
    struct epoll_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
    ev.data.fd = fd;
    if (epoll_ctl(epollFd, EPOLL_CTL_ADD, fd, &ev) == -1)
    {
        if (errno == EEXIST)
        {
            std::cerr << "fd already added: " << fd << std::endl;
            return;
        }
        std::cerr << "epoll_ctl failed for FD " << fd << ": " << strerror(errno) << std::endl;
        throw epollException("❌ Failed to add socket to epoll");
    }
    // std::cout<<"fd added by eppol: "<< fd<<std::endl;
}

int EpollManager::getEpollFd() const 
{
    return epollFd;
}

std::vector<int> EpollManager::waitEvents(Server &obj)
{
    std::vector<int> readyFds;
    const int MAX_EVENTS = 10;
    struct epoll_event events[MAX_EVENTS];
    int n = epoll_wait(epollFd, events, MAX_EVENTS , 1000);
    if (n == -1)
        throw epollException("❌ epoll_wait failed");
    for (int i =0; i < n ; i++)
    {
            int fd = events[i].data.fd;
        if (events[i].events & EPOLLRDHUP)
        {
            std::cout << "Client disconnected fd: " << fd << std::endl;
            obj.closeClient(fd, *this);
            continue;
        }
        // std::cout << " Ready FD: " << events[i].data.fd << std::endl;
        readyFds.push_back(events[i].data.fd);
    }
    return readyFds;    
}

EpollManager::epollException::epollException(const std::string &msg) :_msg(msg){}

EpollManager ::epollException::~epollException() throw() {}

const char* EpollManager::epollException::what() const throw()
{
    return _msg.c_str();
}