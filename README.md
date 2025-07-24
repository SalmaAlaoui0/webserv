# webserv

✅ 1. Initial Step: Add Server Sockets to Epoll
for (size_t i = 0; i < serverSockets.size(); i++)
	epollManager.addSocket(serverSockets[i]);
🟢 Explanation:

Here, we are registering all the server sockets (i.e. the listening sockets, like 127.0.0.1:8080) into the epoll instance so we can be notified when a client tries to connect.

These are not yet clients. These are listening sockets waiting for a client.

✅ 2. Epoll Loop Begins:
while (true) 
{
	std::vector<int> fds = epollManager.waitEvents(*this);
	checkTimeout(clients, epollManager);
🟢 Explanation:

This is the infinite server loop.

epollManager.waitEvents() will block (or wait shortly, depending on the timeout), and return a list of file descriptors (FDs) that had some activity (readable, hang-up, etc.).

✅ 3. For each file descriptor (FD):
for (size_t i = 0; i < fds.size(); ++i) 
{
	if (isServerSocket(fds[i])) 
		acceptNewClient(fds[i], epollManager);
	else 
		handleClient(fds[i], epollManager);
}
🟢 Explanation:

➤ Case 1: isServerSocket(fds[i])
This means the activity is on one of the server listening sockets → a new client is trying to connect.

So you call acceptNewClient() to accept the connection and register the new client socket in epoll.

➤ Case 2: else
This means the activity is from an already accepted client (i.e. a socket where you already accepted a connection).

You call handleClient() to:

Read data.

Process the request.

Send a response.

Or close the socket if needed.

🔁 Summary in Your Words:
Yes, you're close! Let me confirm it for you:

3afaq 3awd chra7 lia hadchi wach flwal we add socket I mean here:
for (size_t i =0; i < serverSockets.size(); i++) epollManager.addSocket(serverSockets[i]);

✅ Yes, these are your listening sockets (e.g. 127.0.0.1:8080), and you register them with epoll so the server can detect when a client tries to connect.

and then we check if found and handle it here:
if (isServerSocket(fds[i])) acceptNewClient(fds[i], epollManager);

✅ Exactly! This is where we detect new client connections and accept them.

and then there is else if not found ??? Is that right!?

✅ Yes again. If the FD isn't a server socket, it must be a client socket (you accepted it before), and now it's ready to read/send data — so you handleClient().

🧠 Fkra Mliha:

Server sockets = listening for connections.

Client sockets = read/write HTTP data.

Epoll helps you manage both efficiently.

If you want, I can walk you through acceptNewClient or handleClient next!





practicing response with a website 

#include <netdb.h>
#include <unistd.h>

std::string fetchFromExampleDotCom()
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return "Failed to create socket";

    struct hostent *host = gethostbyname("example.com");
    if (!host) return "Failed to resolve host";

    struct sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(80);
    std::memcpy(&serverAddr.sin_addr, host->h_addr, host->h_length);

    if (connect(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
        return "Failed to connect";

    const char *httpRequest = "GET / HTTP/1.1\r\nHost: example.com\r\nConnection: close\r\n\r\n";
    send(sock, httpRequest, strlen(httpRequest), 0);

    char buffer[4096];
    std::string result;
    ssize_t bytesRead;
    while ((bytesRead = recv(sock, buffer, sizeof(buffer), 0)) > 0)
        result.append(buffer, bytesRead);

    close(sock);
    return result;
}

void sendResponse(int clientFd, request r)
{
	std::string externalPage = fetchFromExampleDotCom();

	std::cout << "your method is: " << r.get_method() << ", and the path is: " << r.get_path() << ", and version is: " << r.get_version() << std::endl << std::endl;
	// std::ostringstream response;
	// response << "HTTP/1.1 200 OK\r\n"
	// 		 << "Content-Type: text/html\r\n"
	// 		 << "Content-Length: " << externalPage.size() << "\r\n\r\n"
	// 		 << externalPage;


	std::ostringstream response;
	response << externalPage;
	ssize_t sent = send(clientFd, response.str().c_str(), response.str().size(), 0);
	if (sent < 0)
		std::cerr << "❌ send failed: " << strerror(errno) << std::endl;
	else
		std::cout << "Sent external page to FD: " << clientFd << std::endl;
}
