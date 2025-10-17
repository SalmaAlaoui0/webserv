/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EpollManager.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: salaoui <salaoui@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/13 17:09:23 by wzahir            #+#    #+#             */
/*   Updated: 2025/08/29 14:38:33 by salaoui          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#pragma once

#include <iostream>
#include <unistd.h>
#include <sys/epoll.h>
#include <exception>
#include <cstring>
#include <errno.h>
#include <vector>
#include "../includes/Server.hpp"


class Server;

class EpollManager
{
    private :
        int epollFd;
        int MAX_EVENTS;
    public :
        EpollManager();
        ~EpollManager();

        void addSocket(int fd, uint32_t event);
        void modSocket(int fd, uint32_t event);
        void delSocket(int fd);
        int getEpollFd() const;

        std::vector<epoll_event> waitEvents();
        class epollException : public std::exception 
        {
            private:
                std::string _msg;
            public:
                epollException(const std::string &msg);    
                virtual ~epollException() throw();
                virtual const char* what() const throw();
        };
};