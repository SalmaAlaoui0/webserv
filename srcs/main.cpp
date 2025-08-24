/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: wzahir <wzahir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/08 15:24:39 by wzahir            #+#    #+#             */
/*   Updated: 2025/08/22 18:51:34 by wzahir           ###   ########.fr       */
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
        std::cout << "first server's port: " << configs[0].port << std::endl;
        // std::cout << "first server's name: " << configs[0].server_name << std::endl;
        Server server(configs);
        server.run();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return 1;
    }
}

