#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib> 
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>

std::string load_html_file(const std::string& path);


