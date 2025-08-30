#include "../includes/Response.hpp"
#include "../includes/Request.hpp"

extern char **environ;

std::string to_string(size_t size) {
    std::ostringstream oss;
    oss << size;
    return oss.str();
}

std::string execute_cgi(std::string &path, request r, std::string interpreter)
{
    int pipeFD[2];
    if (pipe(pipeFD) == -1)
    {
        std::string HttpHeader = "Cotent-Type: text/plain\r\n\r\n";
        return HttpHeader + "A CGI pipe error\n";
    }
    pid_t pid = fork();
    // std::cout << "HELLLLLLO WORLD\n";
    if (pid == 0)
    {
        dup2(pipeFD[1], STDOUT_FILENO);
        close(pipeFD[0]);
        close(pipeFD[1]);

        setenv("REQUEST_METHOD", "GET", 1);
        setenv("SCRIPT_FILENAME", path.c_str(), 1);
        setenv("QUERY_STRING", r.get_body().c_str(), 1);
		std::string len = to_string(r.get_body().size());
		setenv("CONTENT_LENGTH", len.c_str(), 1);
		setenv("SERVER_PROTOCOL", "HTTP/1.1", 1);
		// so only and only if the request has a body like post we should add the environment
		// variable called "CONTENT_TYPE"


		std::vector<char *> args;
		args.push_back(const_cast<char*>(interpreter.c_str()));
		args.push_back(const_cast<char*>(path.c_str()));
		args.push_back(NULL);
		execve(args[0], &args[0], environ);

        const char *err = "Content-Type: text/plain\r\n\r\nexecve failed\n";
        write(1, err, strlen(err));
        exit (1);
    }
    else
    {
        close(pipeFD[1]);
        std::string scriptContent;
        char buffer[4096];
        ssize_t length;
        while ((length = read(pipeFD[0], buffer, sizeof(buffer))) > 0)
        {
            scriptContent.append(buffer, length);
        }
        close(pipeFD[0]);

        int wstatus;
        waitpid(pid, &wstatus, 0);
        return scriptContent;
    }
}


std::string SendCGIResponse(int clientFd, const std::string &cgi_output, const std::string &default_status = "200 OK")
{
    (void)clientFd;
    (void)cgi_output;
    (void)default_status;



    // 1) split headers / body (support \r\n\r\n and \n\n)
    // size_t header_end = cgi_output.find("\r\n\r\n");
    // size_t header_len = 4;
    // if (header_end == std::string::npos) {
    //     header_end = cgi_output.find("\n\n");
    //     header_len = 2;
    // }

    // std::string header_block;
    // std::string body;
    // if (header_end == std::string::npos) {
    //     std::cout << std::endl << "just telling there is no hearder\n\n\n\n\n";
    //     header_block = "";
    //     body = cgi_output; // no headers from CGI, treat all as body
    // } else {
    //     std::cout << std::endl << "Noooo there is hearder\n\n\n\n\n";
    //     header_block = cgi_output.substr(0, header_end);
    //     std::cout << "and it's >> " << header_block << std::endl;
    //     body = cgi_output.substr(header_end + header_len);
    // }

    // // 2) parse header lines into map
    // std::map<std::string, std::string> headers;
    // std::istringstream hs(header_block);
    // std::string line;
    // std::string status = default_status;

    // while (std::getline(hs, line)) {
    //     if (!line.empty() && line.back() == '\r')
    //         line.pop_back();
    //     if (line.empty()) continue;
    //     size_t colon = line.find(':');
    //     if (colon == std::string::npos) continue; // skip malformed line
    //     std::string name = trim(line.substr(0, colon));
    //     std::string value = trim(line.substr(colon + 1));
    //     // "Status: 404 Not Found" is a special CGI header
    //     if (strcasecmp(name.c_str(), "Status") == 0) {
    //         status = value;
    //     } else {
    //         headers[name] = value;
    //     }
    // }

    // // 3) ensure basic headers
    // if (headers.find("Content-Type") == headers.end())
    //     headers["Content-Type"] = "text/html";

    // headers["Content-Length"] = to_string(body.size());

    // // 4) build HTTP response
    // std::ostringstream response;
    // response << "HTTP/1.1 " << status << "\r\n";
    // for (std::map<std::string, std::string>::iterator it = headers.begin(); it != headers.end(); ++it)
    //     response << it->first << ": " << it->second << "\r\n";
    // response << "\r\n";

    // std::string header_str = response.str();
    // // send header + body
    // ssize_t to_send = (ssize_t)header_str.size() + (ssize_t)body.size();
	// (void)to_send;
    // std::string out = header_str + body;

    // ssize_t sent = send(clientFd, out.c_str(), out.size(), 0);
    // if (sent < 0) {
    //     std::cerr << "❌ send failed: " << strerror(errno) << std::endl;
    //     return "SomeThing went wrong";
    // }
    // std::cout << "✅ CGI response sent to FD: " << clientFd << std::endl;
    return "Done ✅";
}



std::string CheckDirOrFileCGI(std::string requested_path, int clientFd, std::vector<ServerConfig> config, int i, int key, request r)
{
	struct stat statbuf;

    if (stat(requested_path.c_str(), &statbuf) == 0)
	{
        if (S_ISREG(statbuf.st_mode))//Check is a valid file then serve it
		{
			std::string res = execute_cgi(requested_path, r, "/usr/bin/python3");
			return SendCGIResponse(clientFd, res, "200 OK");

        }
		else if (S_ISDIR(statbuf.st_mode)) // if it is a dir attache the index file then serve it
		{
			std::string index_file;
			index_file = join_path(requested_path, config[i].locations[key].index);

			// std::cout << "the index is: " << index_file << "\n\n";
            if (stat(index_file.c_str(), &statbuf) == 0 && S_ISREG(statbuf.st_mode)) // file found ->means everything is good
			{
				std::string ext = index_file.substr(index_file.find_last_of('.'));
				if (ext == ".py" || ext == ".pl" || ext == ".php")
				{
					std::string res = execute_cgi(index_file, r, "/usr/bin/python3");
					std::cout << "\nsalam it's a cgi hereee\n And the output returned is: " << res << std::endl;
					return SendCGIResponse(clientFd, res, "200 OK");
				}
					// execute_cgi(value, r, "/usr/bin/python3");
            // if (stat(index_file.c_str(), &statbuf) == 0 && S_ISREG(statbuf.st_mode)) {
			// 	// std::cout << "the index is: " << index_file << "\n\n";
            }
			else if (config[i].locations[key].autoindex)// Not found pass to autoindex result
			{
				// return serve_autoindex_listing(clientFd, requested_path, uri);
				// return RequestResponse(clientFd, "/home/salaoui/Desktop/webserv/www/autoindex.html");
				// std::cout << "handling autoindex en cours...\n";
				return send_dir_list(clientFd, requested_path);// using requested path only !
            }
			else
			{
				std::cout << "1 Forbidden (index off and no index file) the error is: " << 403 << std::endl;
				return RequestResponse(clientFd, config[i].ErrorPages[403], "403 Forbidden");
            }
        }
		else // If we did attach the file but still it's not found
		{
			return RequestResponse(clientFd, config[i].ErrorPages[403], "403 Forbidden");
			std::cout << "2 Not a regular file or dir the error is: " << 403 << std::endl;
        }
    }
	else
	{
		// return RequestResponse(clientFd, "/home/salaoui/Desktop/webserv/www/404.html");
		std::cout << "3 Not found the error is: " << 404 << std::endl;
		return RequestResponse(clientFd, config[i].ErrorPages[404], "404 Not Found");
        // return send_error(404); // ❌ Not found
    }
	return NULL;
}
