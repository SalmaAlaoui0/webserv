#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib> 
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <algorithm>

std::string ft_content_type(const std::string &headers);
int ft_code_status(std::string headers);
std::string load_html_file(const std::string& path);


