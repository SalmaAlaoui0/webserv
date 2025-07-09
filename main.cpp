/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: wzahir <wzahir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/08 15:24:39 by wzahir            #+#    #+#             */
/*   Updated: 2025/07/08 15:28:03 by wzahir           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>

int main(int argc, char** argv) 
{
    if (argc != 2) 
    {
        std::cerr << "Usage: ./webserv [config file]" << std::endl;
        return 1;
    }
    // Parse config file
    // Start server
    return 0;
}
