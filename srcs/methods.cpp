/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   methods.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: wzahir <wzahir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/28 15:57:20 by wzahir            #+#    #+#             */
/*   Updated: 2025/08/31 21:26:43 by wzahir           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Server.hpp"
#include "../includes/Response.hpp"
#include "../includes/ServerConfig.hpp"
#include "../includes/LocationConfig.hpp"
#include "../includes/ConfigParser.hpp"
#include "../includes/Utils.hpp"


std::vector<std::string> pathchunks(std::string path)
{
	int result;
	std::vector<std::string> chunks;
	std::string word;
	result = path.find('{') != std::string::npos;
	path = path.substr(0, path.find('{'));
	std::istringstream iss(path);
	while (iss >> word)
	{
		chunks.push_back(word);
		// std::cout << "---" << word << "---" << std::endl;
	}
	return chunks;
}

std::string getFileExtension(std::string& path) 
{
    std::size_t lastSlash = path.find_last_of('/');
	if (path[lastSlash + 1] == '\0')
	{
		path = path.substr(0, lastSlash);
    	lastSlash = path.find_last_of('/');
	}

    std::string filename = (lastSlash == std::string::npos) ? path : path.substr(lastSlash + 1);

    if (filename.empty())
        return "";

    std::size_t dotPos = filename.find_last_of('.');

    if (dotPos == std::string::npos || dotPos == 0 || dotPos == filename.size() - 1)
        return "";

    return filename.substr(dotPos + 1);
}

std::string join_path(std::string root, std::string suffix)
{
	std::string fixedRoot = root;
	std::string fixedSuffix = suffix;

	if (!fixedRoot.empty() && fixedRoot[fixedRoot.length() - 1] == '/')
		fixedRoot.erase(fixedRoot.length() - 1);

	if (!fixedSuffix.empty() && fixedSuffix[0] == '/')
		fixedSuffix = fixedSuffix.substr(1);

	return fixedRoot + "/" + fixedSuffix;
}



std::map<int, std::string> getMatchingRootPath(request &r, ServerConfig &config)
{
	std::string requestedPath = r.get_path(); // e.g. "/index.html"
	// std::cout << requestedPath << "!!!!!!!!!!!!!!\n\n" << std::endl;
	std::string matchedRoot;
	size_t maxMatchLength = 0;
	std::string locPath;
	std::vector<std::string> Pchunks;
	size_t i = 0;
	std::string extension;
	size_t coorLoc = 0;

	std::map<int, std::string> result;

	if (!getFileExtension(requestedPath).empty())
	{
		extension = getFileExtension(requestedPath);
		while (i < config.locations.size())
		{
			locPath = config.locations[i].path;
			Pchunks = pathchunks(locPath);
			if (Pchunks.size() == 2 && Pchunks[0] == "~")
			{
				if (Pchunks[1].substr(2) == extension)
				{
					matchedRoot = config.locations[i].root + requestedPath;
					result[i] = matchedRoot;
					return result;
				}
			}
			i++;
		}
	}
	i = 0;
	while (i < config.locations.size())
	{
		std::string root;
		locPath = config.locations[i].path;
		Pchunks = pathchunks(locPath);
		if (Pchunks.size() == 1)
			locPath = Pchunks[0];
		else
			locPath = Pchunks[1];
		if (locPath[1] == '\0' && requestedPath.empty())
		{
			if (config.locations[i].root.empty())
				root = config.root;
			else
				root = config.locations[i].root;
			result[i] = root;
			return result;
		}
		if (Pchunks.size() == 2 && Pchunks[0] == "^~")
		{
			if (requestedPath.find(locPath) == 0)
			{
				if (config.locations[i].root.empty())
					root = config.root;
				else
					root = config.locations[i].root;
				matchedRoot = join_path(root, requestedPath.substr(locPath.length()));
				result[i] = matchedRoot;
				return result;
			}
		}
		else if (requestedPath.find(locPath) == 0 && locPath.length() > maxMatchLength)
		{
			if (config.locations[i].root.empty())
				root = config.root;
			else
				root = config.locations[i].root;
			maxMatchLength = locPath.length();
			matchedRoot = join_path(root, requestedPath.substr(locPath.length()));
			coorLoc = i;
		}
		i++;
	}
	std::cout << "resutl in find matching is: " << matchedRoot << std::endl;
	result[coorLoc] = matchedRoot;
	return result;
}

bool CheckMethodeIsAllowed(std::string method, std::vector<ServerConfig> _configs, int servernum,  int locationum)
{
	std::vector<std::string>::iterator it;
	it = _configs[servernum].locations[locationum].allowed_methods.begin();
	
	if(it ==  _configs[servernum].locations[locationum].allowed_methods.end())
		return 1;
	else
	{
		while (it != _configs[servernum].locations[locationum].allowed_methods.end())
		{
			if (method == *it)
				return 1;
			++it;
		}
	}
	return 0;
}


void send_dir_list(int clientFd, std::string requested_path, std::map<int, Client> &clientobj)
{
	std::stringstream body;
	DIR* directory = opendir(requested_path.c_str());
	struct dirent *entry;

	clientobj[clientFd].autoindex = 1;
    clientobj[clientFd].ResponseChunked = 1;
	if (!directory)
	{
		body << "<html><body><h1>Unable to open director" << requested_path << "</h1></body></html>";
	}
	else
	{
		body << "<html><body><h1>AutoIndex ON: Listing Path files and directories Of" << requested_path << "!</h1><ul>";
		while ((entry = readdir(directory)) != NULL)
		{
			std::string name = entry->d_name;
			std::string new_path = requested_path;
			if (requested_path[requested_path.size() - 1] != '/')
				new_path += "/";
			new_path += name;
			body << "<li><a href=\"" << new_path << "\">" << name << "</a></li>";
			// std::cout << new_path << std::endl;
		}
		closedir(directory);
		body << "</ul></body></html>";
	}
	clientobj[clientFd].autoIndexBody = body.str();
}

std::string to_string98(size_t value) {
    std::ostringstream oss;
    oss << value;
    return oss.str();
}


// std::string execute_cgi(std::string &path, request r, std::string interpreter)

std::string execute_cgi(int clientFd, std::map<int, Client> &clientobj, std::string const &path, request &r, std::string interpreter, EpollManager &epollManager, std::vector<ServerConfig> config, int i)
{
	// clientobj[clientFd].ResponseChunked = 1;
	(void)r;
	(void)i;
	(void)config;
	clientobj[clientFd].has_cgi = 1;
	std::cout << "HEllllllllllllllllllllllllllo world it's a cgi script\n\n";
    int pipeFD[2];
    if (pipe(pipeFD) == -1)
    {
        std::string HttpHeader = "Cotent-Type: text/plain\r\n\r\n";
        std::string str =  HttpHeader + "A CGI pipe error\n";
    }
    pid_t pid = fork();
    if (pid == 0)
    {
        dup2(pipeFD[1], STDOUT_FILENO);
        close(pipeFD[0]);
        close(pipeFD[1]);
		std::string new_body;
		if(clientobj[clientFd].method == "GET")
				setenv("REQUEST_METHOD", "GET", 1);
		if(clientobj[clientFd].method == "POS")
				setenv("REQUEST_METHOD", "POST", 1);
			std::string len;
			if(clientobj[clientFd].chnked)
			{
				len = to_string98(clientobj[clientFd].body_chunked.size());
				new_body = clientobj[clientFd].body_chunked;
			}
			else
			{
				len =  to_string98(clientobj[clientFd].PostBody.size());
				new_body = clientobj[clientFd].PostBody;
			}
        setenv("SCRIPT_FILENAME", path.c_str(), 1);
        setenv("QUERY_STRING", clientobj[clientFd].QUERY_STRING.c_str(), 1);
		setenv("CONTENT_LENGTH", len.c_str(), 1);
		setenv("SERVER_PROTOCOL", "HTTP/1.1", 1);
		setenv("CONTENT_TYPE", clientobj[clientFd].ContentType.c_str(), 1);
		// so only and only if the request has a body like post we should add the environment variable called "CONTENT_TYPE"
		std::vector<char *> args;
		// args.push_back(const_cast<char*>(interpreter.c_str()));
		args.push_back(const_cast<char*>(path.c_str()));
		args.push_back(const_cast<char*>(path.c_str()));
		args.push_back(NULL);
		execve(interpreter.c_str(), &args[0], environ);
		// execve(interpreter.c_str(), &args[0], environ);
		// clientobj[clientFd].cgi_has_problem = 1;
		// clientobj[clientFd].response = Response::buildResponse(r, 500, "Internal Server Error", config[i].ErrorPages[500], clientFd, clientobj);
	
        exit (1);
    }
    else
    {
        close(pipeFD[1]);
		clientobj[clientFd].ResponseChunked = 0;
		fcntl(pipeFD[0], F_SETFL, O_NONBLOCK);

		epollManager.addSocket(pipeFD[0], EPOLLIN);

        CgiInfo info;
        info.pipefd = pipeFD[0];
        info.pid = pid;
        clientobj[clientFd].cgiMap[clientFd] = info;
	}
	// 	char buf[4096];
	// ssize_t n = read(pipeFD[0], buf, sizeof(buf));

	// if (n > 0) {
	// 	std::cout << "Read " << n << " bytes from CGI pipe: n	" << pipeFD[0] << "\n";

	// 	// Print raw bytes (safe even if data is binary)
	// 	std::cout.write(buf, n);
	// 	std::cout << "\n--- END OF PIPE CONTENT ---\n";
	// } 
	// else
	// {
	// 	std::cout << "Some error accured " << std::endl;
	// 	exit (34);
	// }
        // epoll_ctl();
    // }
    return ("hi world\n");
}


std::string find_matching_inter(std::string ext, std::vector<ServerConfig> config, int i, int key)
{
	std::map<std::string, std::string> CgiMap = config[i].locations[key].cgi_pass;
	std::map<std::string, std::string>::iterator it = CgiMap.begin();
	while (it != CgiMap.end())
	{
		if (it->first == ext)
			return(it->second);
		it++;
	}
	return "";
}


void Server::CheckDirOrFile(std::string requested_path, int clientFd, std::vector<ServerConfig> config, int i, int key, request &r, std::map<int, Client> &clientobj, EpollManager &epoll)
{
	struct stat statbuf;
	std::string ext;
	std::string interpreter;
	// std::cout << "----->the requested path is: " << requested_path << std::endl;
    if (stat(requested_path.c_str(), &statbuf) == 0)
	{
        if (S_ISREG(statbuf.st_mode))//Check is a valid file then serve it
		{
			ext = requested_path.substr(requested_path.find_last_of('.'));
			interpreter = find_matching_inter(ext, config, i, key);
			if (!interpreter.empty())
				execute_cgi(clientFd, clientobj, requested_path, r, interpreter, epoll, config, i);
			else
				clients[clientFd].response  = clients[clientFd].response.buildResponse(r, 200, "OK", requested_path, clientFd, clientobj);
		}
		else if (S_ISDIR(statbuf.st_mode)) // if it is a dir attache the index file then serve it
		{
			std::string index_file;
			index_file = join_path(requested_path, config[i].locations[key].index);

			// std::cout << "firstly this hole path is: " << index_file << std::endl;
            if (stat(index_file.c_str(), &statbuf) == 0 && S_ISREG(statbuf.st_mode)) // file found ->means everything is good
			{
				ext = index_file.substr(index_file.find_last_of('.'));
				interpreter = find_matching_inter(ext, config, i, key);
				if (!interpreter.empty())
					execute_cgi(clientFd, clientobj, index_file, r, interpreter, epoll, config, i);
				else
					clients[clientFd].response = clients[clientFd].response.buildResponse(r, 200, "OK", index_file, clientFd, clientobj);
			}
			else if (config[i].locations[key].autoindex)// Not found pass to autoindex result
			{
				send_dir_list(clientFd, requested_path, clientobj);// using requested path only !
			 	clients[clientFd].response = clients[clientFd].response.buildResponse(r, 200, "OK", index_file, clientFd, clientobj);
				// change return value to void and make buildresponse instead of send
            }
			else
			{
				clientobj[clientFd].ResponseChunked = 1;
				clients[clientFd].response = Response::buildResponse(r, 403, "Forbidden",config[i].ErrorPages[403], clientFd, clientobj);
			}
        }
		else // If we did attach the file but still it's not found
		{
			clientobj[clientFd].ResponseChunked = 1;
			clients[clientFd].response = Response::buildResponse(r, 403, "Forbidden",config[i].ErrorPages[403], clientFd, clientobj);
		}
    }
	else
	{
		clientobj[clientFd].ResponseChunked = 1;
		clients[clientFd].response = Response::buildResponse(r, 404, "Not Found",config[i].ErrorPages[404], clientFd, clientobj);
	}
}

void Server::handle_get_methode(request &r, std::vector<ServerConfig> _configs, int clientFd, size_t conf_i, std::map<int, Client> &clientobj, EpollManager &epoll)
{
    if (!clientobj[clientFd].Sending)
	{
		std::map<int, std::string> map;
		map = getMatchingRootPath(r, _configs[conf_i]);
		clientobj[clientFd].GetpathMap = map;
	}
    int key = clientobj[clientFd].GetpathMap.begin()->first;
	if (!_configs[conf_i].locations[key].Return.empty())
	{
		clientobj[clientFd].statusCode = _configs[conf_i].locations[key].Return.begin()->first;
		clientobj[clientFd].statusMsg = "Found";
		clientobj[clientFd].ReturnLocation = _configs[conf_i].locations[key].Return.begin()->second;
	}
    std::string value = clientobj[clientFd].GetpathMap.begin()->second;
	// std::cout << "====>>>>" << value << std::endl;
    if (!CheckMethodeIsAllowed("GET", _configs, conf_i, key))
    {
        clients[clientFd].response = Response::buildResponse(r, 405, "Method Not Allowed",_configs[conf_i].ErrorPages[405], clientFd, clientobj);
        return;
    }
    if (_configs[conf_i].locations[key].Return.empty())
		CheckDirOrFile(value, clientFd, _configs, conf_i, key, r, clientobj, epoll);
}

bool Server::delete_dir_recursive(std::string &path, int clientFd, ServerConfig &config, request &r, std::map<int, Client> clientobj)
{
	DIR *dir = opendir(path.c_str());
	if (!dir)
		return false;
	struct dirent *entry;
	while((entry=readdir(dir)) != NULL)
	{
		std::string name = entry->d_name;
		if(name == "." || name == "..")
			continue;
		std::string fullpath = path + "/" + name;
		struct stat st;
		if(stat(fullpath.c_str(), &st) == 0)
		{
			if (S_ISDIR(st.st_mode))
				delete_dir_recursive(fullpath, clientFd,config, r, clientobj);
			else
				remove(fullpath.c_str());
		}
		else
			clients[clientFd].response = Response::buildResponse(r, 404, "Not Found",config.ErrorPages[404], clientFd, clientobj);
	}
	closedir(dir);
	return remove(path.c_str()) == 0;
}

bool is_directory_empty(const std::string& path)
{
    DIR* dir = opendir(path.c_str());
    if (!dir)
        return false;

    struct dirent* entry;
    int count = 0;
    while ((entry = readdir(dir)) != NULL)
    {
        std::string name = entry->d_name;
        if (name != "." && name != "..")
            count++;
        if (count > 0)
        {
            closedir(dir);
            return false;
        }
    }
    closedir(dir);
    return true;
}

void Server::dir_or_file(std::string &fullpath, int clientFd, ServerConfig &config, request &r, std::map<int, Client> clientobj)
{
	struct stat st;
	if(stat(fullpath.c_str() , &st) != 0)
	{
        clients[clientFd].response = Response::buildResponse(r, 404, "Not Found",config.ErrorPages[404], clientFd, clientobj);
		return ;
	}
	if(S_ISDIR(st.st_mode))
	{
		if (fullpath[fullpath.size() - 1] != '/')
		{
        	clients[clientFd].response = Response::buildResponse(r, 409, "conflict",config.ErrorPages[409], clientFd, clientobj);
			return ;
		}
		if(!is_directory_empty(fullpath))
		{
			if (!delete_dir_recursive(fullpath, clientFd, config, r, clientobj))
			{
				clients[clientFd].response = Response::buildResponse(r, 500, "Internal Server Error",config.ErrorPages[500], clientFd, clientobj);
				return;
			}
			clients[clientFd].response= clients[clientFd].response.buildResponse(r, 204, "No Content",config.ErrorPages[204], clientFd, clientobj);
			return ;
		}
		if(remove(fullpath.c_str()) == 0) //dir empty remove it 
		{
			clients[clientFd].response= clients[clientFd].response.buildResponse(r, 204, "No Content",config.ErrorPages[204], clientFd, clientobj);
			return ;
		}
		else  // Failed to remove directory
		{
			clients[clientFd].response= clients[clientFd].response.buildResponse(r, 500, "Internal Server Error",config.ErrorPages[500], clientFd, clientobj);
			return ;
		}
	}
	else if (S_ISREG(st.st_mode))
	{
		if(access(fullpath.c_str(), W_OK) != 0)  // doesn't have permission for delete
		{
			clients[clientFd].response= clients[clientFd].response.buildResponse(r, 403, "Forbidden",config.ErrorPages[403], clientFd, clientobj);
			return;
		}
		if (std::remove(fullpath.c_str()) == 0)
		{
			clients[clientFd].response= clients[clientFd].response.buildResponse(r, 204, "No Content",config.ErrorPages[204], clientFd, clientobj);
			std::cout << "✅ File deleted: " << fullpath << std::endl;
			return;
		}
		else
		{
			clients[clientFd].response= clients[clientFd].response.buildResponse(r, 500, "Internal Server Error",config.ErrorPages[500], clientFd, clientobj);
			return;
		}
	}
	else //Unsupported file type (e.g., socket, symlink)
	{
		clients[clientFd].response= clients[clientFd].response.buildResponse(r, 403, "Forbidden",config.ErrorPages[403], clientFd, clientobj);
		return;
	}
}
std::string generateId1(size_t length = 16) 
{
    const char set[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    std::string result;
    result.reserve(length);

    for (size_t i = 0; i < length; i++)
        result += set[rand() % (sizeof(set) - 1)];
    return result;
}


void Server::handle_delete_methode(request r, std::vector<ServerConfig> _configs, int clientFd, size_t conf_i, std::map<int, Client> clientobj)
{
	std::map<int, std::string> map;
    map = getMatchingRootPath(r, _configs[conf_i]);
	int key = map.begin()->first;
    if (!CheckMethodeIsAllowed("DELETE", _configs, conf_i, key))
    {
		clients[clientFd].response= clients[clientFd].response.buildResponse(r, 405, "Method Not Allowed", _configs[conf_i].ErrorPages[405], clientFd, clientobj);
        return;
    }
	// if (!_configs[conf_i].locations[key].Return.empty())
	// {
	// 	clientobj[clientFd].statusCode = _configs[conf_i].locations[key].Return.begin()->first;
	// 	clientobj[clientFd].statusMsg = "Found";
	// 	clientobj[clientFd].ReturnLocation = _configs[conf_i].locations[key].Return.begin()->second;
	// 	return;
	// }
    std::string fullpath = map.begin()->second;
    // std::cout << "\nFull path is:" << fullpath << std::endl;
    dir_or_file(fullpath, clientFd, _configs[conf_i], r, clientobj);
}

void Server::handle_post_methode(request & r, std::vector<ServerConfig> _configs, int clientFd, size_t conf_i, std::map<int, Client> &clientobj,EpollManager &epollManager)
{
	std::map<int, std::string> map;/////hereeee
	
    map = getMatchingRootPath(r, _configs[conf_i]);
	int key = map.begin()->first;
    if (!CheckMethodeIsAllowed("POST", _configs, conf_i, key))
    {
		//std::cout << "here internalllll\n";
        //send_response(clientFd, 405, "Method Not Allowed", load_html_file("www/405.html"));
		clients[clientFd].response= Response::buildResponse(r, 405, "Method Not Allowed",_configs[clients[clientFd].conf_i].ErrorPages[405], clientFd, clientobj);
        return;
    }
	// if (!_configs[conf_i].locations[key].Return.empty())
	// {
	// 	clientobj[clientFd].statusCode = _configs[conf_i].locations[key].Return.begin()->first;
	// 	clientobj[clientFd].statusMsg = "Found";
	// 	clientobj[clientFd].ReturnLocation = _configs[conf_i].locations[key].Return.begin()->second;
	// 	return;
	// }
	if (_configs[clients[clientFd].conf_i].locations[map.begin()->first].upload_store.empty())
	{
		//std::cout << "here internalllll\n";
		//send_response(clientFd, 500, "Internal Server Error", load_html_file("www/500.html"));
		clients[clientFd].response= Response::buildResponse(r, 500, "Internal Server Error",_configs[clients[clientFd].conf_i].ErrorPages[500], clientFd, clientobj);
		return ;
	}
    std::string fullpath = map.begin()->second;
	fullpath = join_path(fullpath, _configs[clients[clientFd].conf_i].locations[map.begin()->first].upload_store);
    std::cout << "\nuploading path for post is:" << fullpath << std::endl;
	
	if ((long)r.body.size() > _configs[clients[clientFd].conf_i].client_max_body_size)
	{
		//send_response(clientFd, 413, "Payload Too Large", load_html_file("www/413.html"));
		clients[clientFd].response= Response::buildResponse(r, 413, "Payload Too Large",_configs[clients[clientFd].conf_i].ErrorPages[413], clientFd, clientobj);
		return ;
	}
	
	std::ostringstream filename;
	std::cout << "\nContent Type is: " << clientobj[clientFd].ContentType << std::endl;
	int ext = clientobj[clientFd].ContentType.find('/');
	clientobj[clientFd].ContentType = clientobj[clientFd].ContentType.substr(ext + 1);
	//srand(time(NULL));
	//int i =0;
	filename << fullpath << "/" << generateId1() << "." << clientobj[clientFd].ContentType;
	std::cout << "\nfile is uploaded in: " << filename.str() << std::endl;
	std::ofstream out(filename.str().c_str(),std::ios::binary);
	if(!out)
	{
		std::cout << "yesssss\n0";
		std::cerr << "❌ Failed to open file: " << filename.str() << std::endl;
		clients[clientFd].response= Response::buildResponse(r, 500, "Internal Server Error",_configs[clients[clientFd].conf_i].ErrorPages[500], clientFd, clientobj);
		return;
	}
	// //out << r.body;
	
	// std::cout << "✅✅✅✅✅✅✅✅✅✅" << std::endl;
	if(clientobj[clientFd].chnked ==1 && clientobj[clientFd].body_complete == 1)
	{
		out.write(clientobj[clientFd].body_chunked.c_str(), clientobj[clientFd].body_chunked.size());
	}
	else 
		out.write(clientobj[clientFd].PostBody.c_str(), clientobj[clientFd].PostBody.size());
	if(clientobj[clientFd].cgi_active == 1)
	{
		execute_cgi(clientFd,clientobj,filename.str(), r, "/usr/bin/python3",epollManager, _configs, clients[clientFd].conf_i);
	}
	out.flush();
	out.close();
	std::cout << "&&&&&&&&&&&&&&&&&77"<< _configs[clients[clientFd].conf_i].ErrorPages[201]<< std::endl;
	clients[clientFd].response= Response::buildResponse(r, 201, "Created",_configs[clients[clientFd].conf_i].ErrorPages[201], clientFd, clientobj);
}
