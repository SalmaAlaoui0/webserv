/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: wzahir <wzahir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/08 15:25:50 by wzahir            #+#    #+#             */
/*   Updated: 2025/07/09 12:20:44 by wzahir           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "../includes/Request.hpp"
#include <sstream>
#include<map>

void server()
{
    //✅ 1.Create a socket
    // This creates a file descriptor that represents the server's socket
   // AF_INET → IPv4 ,  SOCK_STREAM → TCP , 0→ default protocol
   int opt = 1;
   int server_fd = socket(AF_INET, SOCK_STREAM, 0);
   setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    // ✅ 2.Bind the socket to an IP and port
    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; // Accept connections from any IP
    address.sin_port = htons(8080);       // Port 8080 (convert to network byte order)

   bind(server_fd, (sockaddr*)&address, sizeof(address));

    //You are saying: "I want to receive messages on any local IP and on port 8080"

    //✅ 3. Listen for incoming connections
    listen(server_fd, 200);
    std::cout << "Server is listening on port 8080...\n";
    //Tells the socket to wait for clients.samah l 5 clients f9a2imat intidar

    //✅ 4. Accept a client connection
   int client_fd = accept(server_fd, NULL, NULL);
    std::cout << "Client connected!\n";
    //This waits until a client connects.It returns a new socket just for communicating with that client.
    //✅ 5. Read and Write data
    //har buffer[1024] = {0};

    //read(client_fd, buffer, 1024);
   
    //std::cout << buffer << std::endl;
   if( parce_request(client_fd)==0)
   {
    std::cout << "------->hi\n";
       const char* response =
           "HTTP/1.1 200 OK\r\n"
           "Content-Type: text/plain\r\n"
           "Content-Length: 13\r\n"
           "\r\n"
           "Hello, World!";
       
       send(client_fd, response, strlen(response), 0);

   }
    //std::cout << "request : " << std::endl << buffer << std::endl;
  
//send(client_fd, msg, strlen(msg), 0);
    
   // ✅ 6. Close the connection
    close(client_fd);
   close(server_fd);

}