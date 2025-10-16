/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EpollManager.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: salaoui <salaoui@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/13 17:11:36 by wzahir            #+#    #+#             */
/*   Updated: 2025/08/29 14:38:13 by salaoui          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "../includes/EpollManager.hpp"

EpollManager::EpollManager()
{
    epollFd = epoll_create(1);
    MAX_EVENTS = 64;
    
    if (epollFd == -1)
        throw epollException("❌ Failed to create epoll instance");
}

EpollManager::~EpollManager()
{
    close(epollFd);
}

void EpollManager::addSocket(int fd, uint32_t event)
{
    struct epoll_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.events = event;
    ev.data.fd = fd;
    if (epoll_ctl(epollFd, EPOLL_CTL_ADD, fd, &ev) == -1)
    {
        std::cerr << "epoll_ctl failed for FD " << fd << ": " << strerror(errno) << std::endl;
        throw epollException("❌epoll_ctl ADD failed");
    }
}

void EpollManager::modSocket(int fd, uint32_t event)
{
    struct epoll_event ev;
    ev.events = event;
    ev.data.fd = fd;
    if (epoll_ctl(epollFd, EPOLL_CTL_MOD, fd, &ev) == -1)
            std::cerr << "epoll_ctl failed for FD " << fd << ": " << strerror(errno) << std::endl;
}

void EpollManager::delSocket(int fd)
{
    if (epoll_ctl(epollFd, EPOLL_CTL_DEL, fd, NULL) == -1)
        throw epollException("❌ epoll_ctl DEL failed");
}

int EpollManager::getEpollFd() const 
{
    return epollFd;
}

std::vector<epoll_event> EpollManager::waitEvents()
{
    std::vector< epoll_event> events(MAX_EVENTS);
    int n = epoll_wait(epollFd, &events[0], MAX_EVENTS , 0);
    if (n == -1)
        throw epollException("❌ epoll_wait failed");
    events.resize(n);
    return events;    
}

EpollManager::epollException::epollException(const std::string &msg) :_msg(msg){}

EpollManager ::epollException::~epollException() throw() {}

const char* EpollManager::epollException::what() const throw()
{
    return _msg.c_str();
}