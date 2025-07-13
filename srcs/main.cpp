/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: salaoui <salaoui@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/08 15:24:39 by wzahir            #+#    #+#             */
/*   Updated: 2025/07/13 18:35:27 by salaoui          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include "../includes/ConfigParser.hpp"
#include "../includes/Server.hpp"


int main(int argc, char** argv) 
{
    ConfigParser obj;
    try
    {
        std::string configFile = "default.conf";
        if (argc == 2)
            configFile = argv[1];
        else if (argc > 2) 
        {
            std::cerr << "Usage: ./webserv [config_file]" << std::endl;
            return 1;
        }
        std::vector<ServerConfig> configs = obj.parseConfig(configFile);
        std::cout << "first server's name: " << configs[0].server_name << std::endl;
        // parseConfig(argv[1]);
        // Server server(configs);
        // server.run();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return 1;
    }
    return 0;
}

