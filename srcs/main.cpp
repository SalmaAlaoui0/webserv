/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: salaoui <salaoui@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/08 15:24:39 by wzahir            #+#    #+#             */
/*   Updated: 2025/07/09 14:31:32 by salaoui          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include "../includes/ConfigParser.hpp"
#include "../includes/Request.hpp"

int main(int argc, char** argv) 
{
    argc =0;
    //(void)argv;
    // if (argc != 2) 
    // {
    //     std::cerr << "Usage: ./webserv [config file]" << std::endl;
    //     return 1;
    // }
    // Parse config file
    // still in the start Not done yet 
    try
    {
        parseConfig(argv[1]);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    server();
    // Start server
    return 0;
}
