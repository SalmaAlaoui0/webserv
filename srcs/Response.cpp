#include "../includes/Server.hpp"
#include "../includes/Response.hpp"

std::string RequestResponse(int clientFd, std::string filePath)
{
    std::ifstream file(filePath.c_str(), std::ios::in | std::ios::binary);// we open file in binary read mode to support text && binary files
    if (!file) {
        std::cerr << "❌ Failed to open file: " << filePath << std::endl;
        return ("❌ Failed to open file: ");
    }

    std::ostringstream ss; // to put file content in it ;)
    ss << file.rdbuf();
    std::string body = ss.str(); //Then put in in body and it will be in response str

    std::string contentType = "text/plain"; // set a default content type;// then here we specify (it's optional btw)
    if (filePath.find(".html") != std::string::npos) contentType = "text/html";
    else if (filePath.find(".css") != std::string::npos) contentType = "text/css";
    else if (filePath.find(".js") != std::string::npos) contentType = "application/javascript";
    else if (filePath.find(".jpg") != std::string::npos || filePath.find(".jpeg") != std::string::npos) contentType = "image/jpeg";
    else if (filePath.find(".png") != std::string::npos) contentType = "image/png";

    std::ostringstream response;// last but not least ofc building proper HTTP response!
    response << "HTTP/1.1 200 OK\r\n"
             << "Content-Type: " << contentType << "\r\n"
             << "Content-Length: " << body.size() << "\r\n\r\n"
             << body;

    ssize_t sent = send(clientFd, response.str().c_str(), response.str().size(), 0);// Sending it to our lovely client
    if (sent < 0)// send failed!
        std::cerr << "❌ send failed: " << strerror(errno) << std::endl;
    else// Indicating success in our console
    {
        std::cout << "✅ File response sent to FD: " << clientFd << std::endl;
        return "Done ✅";
    }
    return ("SomeThing went wrong");
}
