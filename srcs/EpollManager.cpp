/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EpollManager.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: wzahir <wzahir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/13 17:11:36 by wzahir            #+#    #+#             */
/*   Updated: 2025/07/15 22:27:44 by wzahir           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/EpollManager.hpp"

EpollManager::EpollManager()
{
    epollFd = epoll_create(1024);
    if (epollFd == -1)
        throw epollException("Failed to create epoll instance");
}

EpollManager::~EpollManager()
{
    close(epollFd);
}

void EpollManager::addSocket(int fd)
{
    // std::cout << "[EpollManager] Adding FD to epoll: " << fd << std::endl;
    struct epoll_event ev;
    ev.events = EPOLLIN;// | EPOLLET;
    ev.data.fd = fd;
    if (epoll_ctl(epollFd, EPOLL_CTL_ADD, fd, &ev) == -1)//“Hey epollFd, please start watching fd and tell me when I can read from it 
     {
        if (errno == EEXIST) 
        {
            std::cerr << "[EpollManager] FD already added: " << fd << std::endl;
            return;
        }
        std::cerr << "[EpollManager] epoll_ctl failed for FD " << fd
        << ": " << strerror(errno) << std::endl;
        throw epollException("Failed to add socket to epoll");
     }
}

int EpollManager::getEpollFd() const 
{
    return epollFd;
}

std::vector<int> EpollManager::waitEvents(int timeout)
{
    std::vector<int> readyFds;
    const int MAX_EVENTS = 1064;
    struct epoll_event events[MAX_EVENTS];
    int n = epoll_wait(epollFd, events, MAX_EVENTS , timeout);
    if (n == -1)
        throw epollException("epoll_wait failed");
    std::cout << "epoll_wait  " << n << " events\n";
    for (int i =0; i < n ; i++)
    {
        std::cout << "  - Ready FD: " << events[i].data.fd << std::endl;
        std::cout << "  event: " << events[i].events << std::endl;
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