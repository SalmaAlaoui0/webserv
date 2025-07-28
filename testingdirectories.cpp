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














/// AUTOINDEX IS ON ---->

// Helper function to generate autoindex HTML
// std::string generate_autoindex_html(std::string path, std::string uri) {
//     std::stringstream html;
//     html << "<html><head><title>Index of " << uri << "</title></head><body>";
//     html << "<h1>Index of " << uri << "</h1><ul>";

//     DIR* dir = opendir(path.c_str());
//     if (!dir) {
//         html << "<li>Unable to open directory</li>";
//     } else {
//         struct dirent* entry;
//         while ((entry = readdir(dir)) != NULL) {
//             std::string name = entry->d_name;
//             // Skip '.' to avoid self link loops, optional
//             if (name == ".") continue;

//             std::string link = uri;
//             if (!link.empty() && link[link.size() - 1] != '/')
//     			link += "/";
//             link += name;

//             html << "<li><a href=\"" << link << "\">" << name << "</a></li>";
//         }
//         closedir(dir);
//     }

//     html << "</ul></body></html>";
//     return html.str();
// }

// // Function to send autoindex HTML response
// std::string serve_autoindex_listing(int clientFd, std::string requested_path, std::string uri) {
//     std::string body = generate_autoindex_html(requested_path, uri);

//     std::stringstream response;
//     response << "HTTP/1.1 200 OK\r\n";
//     response << "Content-Type: text/html\r\n";
//     response << "Content-Length: " << body.size() << "\r\n";
//     response << "Connection: close\r\n";
//     response << "\r\n";
//     response << body;

//     // send the response string via your socket to clientFd
//     // You probably have a send function; let's say:
//     send(clientFd, response.str().c_str(), response.str().size(), 0);

//     return ""; // or some status string
// }

