#include <iostream>
#include <sys/stat.h>
#include <string>

void parsePath(const std::string& path, std::string& dirPath, std::string& fileName) {
    struct stat pathStat;

    // Remove trailing slash for stat to work properly on files
    std::string cleanPath = path;
    // std::cout << "The path was: " << cleanPath << std::endl;
    if (cleanPath.size() > 1 && cleanPath.back() == '/')
        cleanPath.pop_back();
    // std::cout << "The path become: " << cleanPath << std::endl;

    // Try to stat the path
    if (stat(cleanPath.c_str(), &pathStat) != 0) {
        perror("stat");
        dirPath = "";
        fileName = "";
        return;
    }

    if (S_ISDIR(pathStat.st_mode)) {
        dirPath = path;
        fileName = "";
    } else if (S_ISREG(pathStat.st_mode)) {
        // It's a regular file → extract file name & dir
        size_t lastSlash = cleanPath.find_last_of('/');
        if (lastSlash != std::string::npos) {
            dirPath = cleanPath.substr(0, lastSlash);
            fileName = cleanPath.substr(lastSlash + 1);
        } else {
            dirPath = ".";
            fileName = cleanPath;
        }
    } else {
        // Something else (symlink, device file, etc.)
        std::cout << "helllo" << std::endl;
        dirPath = "";
        fileName = "";
    }
}

int main() {
    std::string input = "/home/salaoui/Desktop/webserv/www/404.html/";
    std::string dirPath, fileName;

    parsePath(input, dirPath, fileName);

    std::cout << "Directory path: " << dirPath << std::endl;
    std::cout << "File name: " << (fileName.empty() ? "NULL" : fileName) << std::endl;

    return 0;
}

