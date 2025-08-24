#include "../includes/Server.hpp"
#include "../includes/Response.hpp"

std::string send_video(int clientFd, std::string filePath, std::string resCode)
{

    // std::ifstream file(filePath, std::ios::binary);
    std::ifstream file(filePath);
    if (!file) {
        std::cerr << "❌ Failed to open file\n";
        return "Failed";
    }

    file.seekg(0, std::ios::end);
    size_t filesize = file.tellg();
    file.seekg(0, std::ios::beg);

    std::cout << "\n\n----Hello world----\n\n";
    std::string contentType = "video/mp4";
    std::ostringstream headers;
    headers << "HTTP/1.1 " << resCode << "\r\n"
            << "Content-Type: " << contentType << "\r\n"
            << "Content-Length: " << filesize << "\r\n"
            << "Connection: close\r\n\r\n";

    std::cout << "\n\n----Hello world---lastly-\n\n";

    std::string headerStr = headers.str();
    send(clientFd, headerStr.c_str(), headerStr.size(), 0);

    const size_t bufsize = 1024;
    char buffer[bufsize];
    int i = 0;
    while (file.read(buffer, bufsize) || file.gcount() > 0) 
    {
        std::cout << "\n\n----Hello world  " << i << "----\n\n";
        if (send(clientFd, buffer, file.gcount(), 0) == -1)
        {
            perror("send failed");
            std::cout << "send failed here" << i << "------\n\n";
            break; // Or handle error properly
        }      
        i++;
    }

//     const size_t bufsize = 8192;
//     char buffer[bufsize];
//     int i = 0;

//     while (file.read(buffer, bufsize) || file.gcount() > 0) {
//         size_t toSend = file.gcount();
//         size_t sent = 0;

//         while (sent < toSend) {
//             ssize_t n = send(clientFd, buffer + sent, toSend - sent, 0);
//             if (n == -1) {
//                 if (errno == EINTR) {
//                     // Interrupted by signal, retry
//                     continue;
//                 } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
//                     // Socket buffer full → wait until writable
//                     fd_set wfds;
//                     FD_ZERO(&wfds);
//                     FD_SET(clientFd, &wfds);

//                     // wait (blocking) until socket is ready
//                     if (select(clientFd + 1, NULL, &wfds, NULL, NULL) > 0) {
//                         continue; // try again
//                     } else {
//                         perror("select failed");
//                         return "Failed";
//                     }
//                 } else {
//                     perror("send failed");
//                     return "Failed";
//                 }
//             }
//             sent += n; // advance pointer by the number of bytes actually sent
//         }

//         std::cout << "✅ Sent chunk " << i << " (" << toSend << " bytes)\n";
//         i++;
// }

    std::cout << "✅ File sent\n";
    return "Done";
}

std::string RequestResponse(int clientFd, std::string filePath, std::string resCode)
{
    std::ifstream file(filePath.c_str(), std::ios::in | std::ios::binary);// we open file in binary read mode to support text && binary files
    if (!file)
    {
        std::cerr << "❌ Failed to open file: " << filePath << std::endl;
        return ("❌ Failed to open file: ");
    }

    std::ostringstream ss; // to put file content in it ;)
    ss << file.rdbuf();
    std::string body = ss.str(); //Then put in in body and it will be in response str

    std::string contentType = "text/plain"; // set a default content type;// then here we specify (it's optional btw)
    if (filePath.find(".html") != std::string::npos)
        contentType = "text/html";
    else if (filePath.find(".css") != std::string::npos)
        contentType = "text/css";
    else if (filePath.find(".js") != std::string::npos)
        contentType = "application/javascript";
    else if (filePath.find(".jpg") != std::string::npos || filePath.find(".jpeg") != std::string::npos)
        contentType = "image/jpeg";
    else if (filePath.find(".png") != std::string::npos)
        contentType = "image/png";
    else if (filePath.find(".mp4") != std::string::npos)
        return send_video(clientFd, filePath, "200 OK");

    std::ostringstream response;// last but not least ofc building proper HTTP response!
    response << "HTTP/1.1 " << resCode << "\r\n"
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
