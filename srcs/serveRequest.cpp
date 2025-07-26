#include "../includes/Server.hpp"
#include "../includes/Response.hpp"

std::string send_file_response(int clientFd, std::string filePath)
{
    std::ifstream file(filePath.c_str(), std::ios::in | std::ios::binary);
    if (!file) {
        std::cerr << "❌ Failed to open file: " << filePath << std::endl;
        return ("❌ Failed to open file: ");
    }

    // Read entire file content
    std::ostringstream ss;
    ss << file.rdbuf();
    std::string body = ss.str();

    // (Optional) Detect content-type from file extension
    std::string contentType = "text/plain";
    if (filePath.find(".html") != std::string::npos) contentType = "text/html";
    else if (filePath.find(".css") != std::string::npos) contentType = "text/css";
    else if (filePath.find(".js") != std::string::npos) contentType = "application/javascript";
    else if (filePath.find(".jpg") != std::string::npos || filePath.find(".jpeg") != std::string::npos) contentType = "image/jpeg";
    else if (filePath.find(".png") != std::string::npos) contentType = "image/png";

    // Compose response
    std::ostringstream response;
    response << "HTTP/1.1 200 OK\r\n"
             << "Content-Type: " << contentType << "\r\n"
             << "Content-Length: " << body.size() << "\r\n\r\n"
             << body;

    // Send it
    ssize_t sent = send(clientFd, response.str().c_str(), response.str().size(), 0);
    if (sent < 0)
        std::cerr << "❌ send failed: " << strerror(errno) << std::endl;
    else
    {
        std::cout << "✅ File response sent to FD: " << clientFd << std::endl;
        return "Done ✅";
    }
    return ("SomeThing went wrong");
}
