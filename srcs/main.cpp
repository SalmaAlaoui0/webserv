/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: wzahir <wzahir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/08 15:24:39 by wzahir            #+#    #+#             */
/*   Updated: 2025/07/17 14:30:50 by wzahir           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include "../includes/ConfigParser.hpp"
#include "../includes/Request.hpp"
#include "../includes/Server.hpp"

int main(int argc, char** argv) 
{
    ConfigParser obj;
    try
    {
        std::string configFile = "conf/default.conf";
        if (argc == 2)
            configFile = argv[1];
        else if (argc > 2) 
        {
            std::cerr << "Usage: ./webserv [config_file]" << std::endl;
            return 1;
        }
        std::vector<ServerConfig> configs = obj.parseConfig(configFile);
        std::cout << "first server's name: " << configs[0].server_name << std::endl;
        Server server(configs);
        server.run();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return 1;
    }
    return 0;
}


// #include <iostream>
// #include <unistd.h>
// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <fcntl.h>
// #include <sys/epoll.h>

// int main() {
//     int server_fd = socket(AF_INET, SOCK_STREAM, 0);
//     fcntl(server_fd, F_SETFL, O_NONBLOCK);

//     sockaddr_in addr{};
//     addr.sin_family = AF_INET;
//     addr.sin_port = htons(8080);
//     addr.sin_addr.s_addr = INADDR_ANY;
//     bind(server_fd, (sockaddr*)&addr, sizeof(addr));
//     listen(server_fd, 10);

//     int epoll_fd = epoll_create1(0);
//     epoll_event ev{};
//     ev.events = EPOLLIN;
//     ev.data.fd = server_fd;
//     epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev);

//     epoll_event events[10];

//     while (true) {
//         int n = epoll_wait(epoll_fd, events, 10, -1);
//         for (int i = 0; i < n; ++i) {
//             if (events[i].data.fd == server_fd) {
//                 int client_fd = accept(server_fd,NULL,NULL);
//                 fcntl(client_fd, F_SETFL, O_NONBLOCK);
//                 epoll_event client_ev{};
//                 client_ev.events = EPOLLIN;
//                 client_ev.data.fd = client_fd;
//                 epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &client_ev);
//                 std::cout << "✅ New client " << client_fd << std::endl;
//             } else {
//                 char buf[1024];
//                 int bytes = read(events[i].data.fd, buf, sizeof(buf));
//                 if (bytes <= 0) {
//                     std::cout << "❌ Client disconnected " << events[i].data.fd << std::endl;
//                     close(events[i].data.fd);
//                 } else {
//                     std::cout << "📥 Received: " << std::string(buf, bytes) << std::endl;
//                     std::string response = "HTTP/1.1 200 OK\r\nContent-Length: 12\r\n\r\nHello world";
//                     send(events[i].data.fd, response.c_str(), response.size(), 0);
//                 }
//             }
//         }
//     }
// }


