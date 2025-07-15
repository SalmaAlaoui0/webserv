/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EpollManager.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: wzahir <wzahir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/13 17:09:23 by wzahir            #+#    #+#             */
/*   Updated: 2025/07/15 15:43:44 by wzahir           ###   ########.fr       */
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

class EpollManager
{
    private :
        int epollFd;
    public :
        EpollManager();
        ~EpollManager();

        void addSocket(int fd);
        int getEpollFd() const;
        std::vector<int> waitEvents(int time);
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