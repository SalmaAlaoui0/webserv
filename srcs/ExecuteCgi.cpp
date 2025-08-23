#include "../includes/Response.hpp"
#include "../includes/Request.hpp"
#include "../includes/Response.hpp"
#include "../includes/Request.hpp"


extern char **environ; // for execve

std::string execute_cgi(const std::string &script_path, request &r, const std::string &interpreter)
{
    int pipe_fd[2];
    if (pipe(pipe_fd) == -1) {
        // return a small HTTP-like payload so SendCGIResponse can handle it
        return std::string("Content-Type: text/plain\r\n\r\n") + "CGI pipe error\n";
    }

    pid_t pid = fork();
    if (pid < 0) {
        close(pipe_fd[0]); close(pipe_fd[1]);
        return std::string("Content-Type: text/plain\r\n\r\n") + "CGI fork error\n";
    }

    if (pid == 0)
    {
        // --- child ---
        // send child's stdout to parent through pipe
        dup2(pipe_fd[1], STDOUT_FILENO);
        // optionally redirect stderr to stdout if you want script errors in output:
        // dup2(pipe_fd[1], STDERR_FILENO);

        close(pipe_fd[0]);
        close(pipe_fd[1]);

        // set env vars for CGI (GET)
        setenv("REQUEST_METHOD", "GET", 1);
        setenv("QUERY_STRING", r.get_body().c_str(), 1);
        setenv("CONTENT_LENGTH", "0", 1);
        setenv("SCRIPT_FILENAME", script_path.c_str(), 1);// script path
        setenv("SERVER_PROTOCOL", "HTTP/1.1", 1);// which protocol

        // SERVER_NAME and SERVER_PORT should be taken from the request/config
        // Example:
        setenv("SERVER_NAME", "localhost", 1);
        {
            std::ostringstream portoss; portoss << r.get_final_port(r);
            setenv("SERVER_PORT", portoss.str().c_str(), 1);
        }

        // build args for execve: interpreter + script_path
        std::vector<char*> args;
        args.push_back(const_cast<char*>(interpreter.c_str()));
        args.push_back(const_cast<char*>(script_path.c_str()));
        args.push_back(NULL);

        // execve — pass current environ so standard env is kept
        execve(args[0], &args[0], environ);

        // if execve fails:
        const char *err = "Content-Type: text/plain\r\n\r\nexecve failed\n";
        write(STDOUT_FILENO, err, strlen(err));
        _exit(1);
    }
    else
    {
        // --- parent ---
        close(pipe_fd[1]);
        std::string output;
        char buf[4096];
        ssize_t n;
        while ((n = read(pipe_fd[0], buf, sizeof(buf))) > 0)
        {
            output.append(buf, n);
        }
        close(pipe_fd[0]);

        int wstatus = 0; waitpid(pid, &wstatus, 0);
        // optionally inspect wstatus for exit code

        return output; // raw CGI stdout (usually contains headers + body)
    }
}


std::string SendCGIResponse(int clientFd, const std::string &cgi_output, const std::string &default_status = "200 OK")
{
    // 1) split headers / body (support \r\n\r\n and \n\n)
    size_t header_end = cgi_output.find("\r\n\r\n");
    size_t header_len = 4;
    if (header_end == std::string::npos) {
        header_end = cgi_output.find("\n\n");
        header_len = 2;
    }

    std::string header_block;
    std::string body;
    if (header_end == std::string::npos) {
        std::cout << std::endl << "just telling there is no hearder\n\n\n\n\n";
        header_block = "";
        body = cgi_output; // no headers from CGI, treat all as body
    } else {
        std::cout << std::endl << "Noooo there is hearder\n\n\n\n\n";
        header_block = cgi_output.substr(0, header_end);
        std::cout << "and it's >> " << header_block << std::endl;
        body = cgi_output.substr(header_end + header_len);
    }

    // 2) parse header lines into map
    std::map<std::string, std::string> headers;
    std::istringstream hs(header_block);
    std::string line;
    std::string status = default_status;

    while (std::getline(hs, line)) {
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        if (line.empty()) continue;
        size_t colon = line.find(':');
        if (colon == std::string::npos) continue; // skip malformed line
        std::string name = trim(line.substr(0, colon));
        std::string value = trim(line.substr(colon + 1));
        // "Status: 404 Not Found" is a special CGI header
        if (strcasecmp(name.c_str(), "Status") == 0) {
            status = value;
        } else {
            headers[name] = value;
        }
    }

    // 3) ensure basic headers
    if (headers.find("Content-Type") == headers.end())
        headers["Content-Type"] = "text/html";

    headers["Content-Length"] = std::to_string(body.size());

    // 4) build HTTP response
    std::ostringstream response;
    response << "HTTP/1.1 " << status << "\r\n";
    for (std::map<std::string, std::string>::iterator it = headers.begin(); it != headers.end(); ++it)
        response << it->first << ": " << it->second << "\r\n";
    response << "\r\n";

    std::string header_str = response.str();
    // send header + body
    ssize_t to_send = (ssize_t)header_str.size() + (ssize_t)body.size();
	(void)to_send;
    std::string out = header_str + body;

    ssize_t sent = send(clientFd, out.c_str(), out.size(), 0);
    if (sent < 0) {
        std::cerr << "❌ send failed: " << strerror(errno) << std::endl;
        return "SomeThing went wrong";
    }
    std::cout << "✅ CGI response sent to FD: " << clientFd << std::endl;
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
