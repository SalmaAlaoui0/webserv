#include "../includes/Utils.hpp"

std::string load_html_file(const std::string& path)
{
    std::ifstream file(path.c_str());
    if (!file)
        return "";

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}


std::string ft_content_type(const std::string &headers)
{
    std::istringstream stream(headers);
    std::string line;
    while (std::getline(stream, line)) {
        if (line.find("Content-Type:") != std::string::npos)
        {
            size_t pos = line.find(":");
            if (pos != std::string::npos) {
                std::string contentType = line.substr(pos + 1);
                contentType.erase(0, contentType.find_first_not_of(" \t\r\n"));
                contentType.erase(contentType.find_last_not_of(" \t\r\n") + 1);
                return contentType;
            }
        }
    }
    return "text/plain"; // Default content-type
}

int ft_code_status(std::string headers)
{
    std::istringstream stream(headers);
    std::string line;

    while (std::getline(stream, line)) {
        if (line.find("Status:") != std::string::npos)
        {
            size_t pos = line.find(":");
            if (pos != std::string::npos)
            {
                std::string statusCodeStr = line.substr(pos + 1);
                size_t start = statusCodeStr.find_first_not_of(" \t\r\n");
                if (start != std::string::npos)
                    statusCodeStr = statusCodeStr.substr(start);
                size_t end = statusCodeStr.find_last_not_of(" \t\r\n");
                if (end != std::string::npos)
                    statusCodeStr = statusCodeStr.substr(0, end + 1);
                std::stringstream ss(statusCodeStr);
                int code;
                if (ss >> code)
                    return code;
                else
                    return 200;
            }
        }
    }
    return 200; // Default st code
}

