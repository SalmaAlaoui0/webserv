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
#include <sstream>
#include <sys/stat.h>

std::vector<std::string> pathchunks(std::string path)
{
	int result;
	std::vector<std::string> chunks;
	std::string word;
	result = path.find('{') != std::string::npos;
	path = path.substr(0, path.find('{'));
	std::istringstream iss(path);
	while (iss >> word)
		chunks.push_back(word);
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

std::vector<std::string> splitPath(const std::string &path) {
    std::vector<std::string> parts;
    std::stringstream ss(path);
    std::string part;
    while (std::getline(ss, part, '/')) {
        if (!part.empty()) parts.push_back(part);
    }
    return parts;
}

std::string joinPath(const std::vector<std::string> &parts, bool Slashfound) {
    std::string result = "";
    for (std::vector<std::string>::const_iterator it = parts.begin(); it != parts.end(); ++it) {
		if (result.empty() && !Slashfound)
        	result = *it;
		else
        	result += "/" + *it;
    }
    return result.empty() ? "/" : result;
}

std::string mergePaths(std::string root, std::string request) 
{
    std::vector<std::string> rootParts = splitPath(root);
    std::vector<std::string> reqParts = splitPath(request);
	bool slashfound = 0;
	if (root[0] == '/')
		slashfound = 1;
    while (!rootParts.empty() && !reqParts.empty() && 
           rootParts.back() == reqParts.front())
	{
        reqParts.erase(reqParts.begin());
    }

    std::vector<std::string> merged = rootParts;
    merged.insert(merged.end(), reqParts.begin(), reqParts.end());
    return joinPath(merged, slashfound);
}

std::map<int, std::string> getMatchingRootPath(request &r, ServerConfig &config)
{
	std::string requestedPath = r.get_path();
	// if (requestedPath[requestedPath.size() - 1] == '/')
	// 	r.slash = 1;
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
				matchedRoot = mergePaths(root, requestedPath);
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
			matchedRoot = mergePaths(root, requestedPath);
			coorLoc = i;
		}
		i++;
	}
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

void execute_cgi(int clientFd, std::map<int, Client> &clientobj, std::string const &path, std::string interpreter, EpollManager &epollManager)
{
	clientobj[clientFd].has_cgi = 1;
	Server s;

	int input_pipe[2];
	int pipeFD[2];
	if (pipe(input_pipe) == -1 || pipe(pipeFD) == -1)
	{
		std::cerr<< "pipe failed \n";
		return;
	}

	pid_t pid = fork();
	if (pid == -1)
	{
		std::cerr<< "fork failed \n";
		return;
	}
	if (pid == 0)
	{
		dup2(input_pipe[0], STDIN_FILENO);
		dup2(pipeFD[1], STDOUT_FILENO);
		close(input_pipe[0]);
		close(input_pipe[1]);
		close(pipeFD[0]);
		close(pipeFD[1]);
		std::vector<char *> args;
		args.push_back(const_cast<char*>(interpreter.c_str()));
		args.push_back(const_cast<char*>(path.c_str()));
		args.push_back(NULL);
		if (clientobj[clientFd].method == "POST")
		{
			setenv("REQUEST_METHOD", "POST", 1);
			std::string body = "";
			if (clientobj[clientFd].chnked)
				body = clientobj[clientFd].body_chunked;
			else
				body = clientobj[clientFd].PostBody;
			std::string len = to_string98(body.size());
			setenv("CONTENT_LENGTH", len.c_str(), 1);
			setenv("CONTENT_TYPE", clientobj[clientFd].ContentType.c_str(), 1);
		}
		else
		{
			setenv("REQUEST_METHOD", "GET", 1);
			setenv("CONTENT_LENGTH", "0", 1);
		}
		setenv("CONTENT_TYPE", clientobj[clientFd].ContentType.c_str(), 1);
		setenv("SCRIPT_FILENAME", path.c_str(), 1);
		setenv("QUERY_STRING", clientobj[clientFd].QUERY_STRING.c_str(), 1);
		setenv("SERVER_PROTOCOL", "HTTP/1.1", 1);
		setenv("REDIRECT_STATUS", "200", 1);
		execve(interpreter.c_str(), &args[0], environ);
		std::cerr<<"execve failed "<<strerror(errno)<<std::endl;
		exit(1);
	}
	else
	{
		close(input_pipe[0]); 
		close(pipeFD[1]);   
		close(input_pipe[1]);
		if (fcntl(pipeFD[0], F_SETFL, O_NONBLOCK) < 0)
			std::cerr<<"fcntl pipe failed \n";
		epollManager.addSocket(pipeFD[0], EPOLLIN);
		CgiInfo info;
		info.pipefd = pipeFD[0];
		info.pid = pid;
		info.start = time(NULL);
		clientobj[clientFd].cgiMap[clientFd] = info;
	}
}


std::string find_matching_inter(std::string ext, std::vector<ServerConfig> config, int i, int key)
{
	std::map<std::string, std::string> CgiMap = config[i].locations[key].cgi_pass;
	std::map<std::string, std::string>::iterator it = CgiMap.begin();
	if (ext != "")
	{
		while (it != CgiMap.end())
		{
			if (it->first == ext)
				return(it->second);
			it++;
		}
	}
	return "";
}


void Server::CheckDirOrFile(std::string requested_path, int clientFd, std::vector<ServerConfig> config, int i, int key, std::map<int, Client> &clientobj, EpollManager &epoll)
{
	struct stat statbuf;
	std::string ext;
	std::string interpreter;
	bool indexNotFound = false;
    if (stat(requested_path.c_str(), &statbuf) == 0)
	{
        if (S_ISREG(statbuf.st_mode))//Check is a valid file then serve it
		{
			size_t dot_pos = requested_path.find_last_of('.');

			if (dot_pos != std::string::npos)
			{
				ext = requested_path.substr(dot_pos);
				interpreter = find_matching_inter(ext, config, i, key);
				ext = requested_path.substr(requested_path.find_last_of('.'));
			}
			else
				ext = "";
			if (!interpreter.empty())
				execute_cgi(clientFd, clientobj, requested_path, interpreter, epoll);
			else
				clients[clientFd].response  = clients[clientFd].response.buildResponse(200, "OK", requested_path, clientFd, clientobj, _configs);
		}
		else if (S_ISDIR(statbuf.st_mode)) // if it is a dir attache the index file then serve it
		{
			std::string index_file;
			std::string file;
			if (!config[i].locations[key].index.empty())
				file = config[i].locations[key].index;
			else
				file = config[i].index;
			if (config[i].locations[key].index.empty() && config[i].index.empty())
				indexNotFound = true;
			index_file = mergePaths(requested_path, file);
            if (stat(index_file.c_str(), &statbuf) == 0 && S_ISREG(statbuf.st_mode)) // file found ->means everything is good
			{
				ext = index_file.substr(index_file.find_last_of('.'));
				interpreter = find_matching_inter(ext, config, i, key);
				if (!interpreter.empty())
					execute_cgi(clientFd, clientobj, index_file, interpreter, epoll);
				else
					clients[clientFd].response = clients[clientFd].response.buildResponse(200, "OK", index_file, clientFd, clientobj, _configs);
			}
			else if (config[i].locations[key].autoindex && indexNotFound)// Not found pass to autoindex result
			{
				send_dir_list(clientFd, requested_path, clientobj);// using requested path only !
				clients[clientFd].response = clients[clientFd].response.buildResponse(200, "OK", index_file, clientFd, clientobj, _configs);
            }
			else
			{
				clientobj[clientFd].ResponseChunked = 1;
				if (!clientobj[clientFd].has_cookie)
					clients[clientFd].response = Response::buildResponse(403, "Forbidden", "www/new_client.html", clientFd, clientobj, _configs);
				else
					clients[clientFd].response = Response::buildResponse(403, "Forbidden", "www/returning_client.html", clientFd, clientobj, _configs);
			}
        }
		else // If we did attach the file but still it's not found
		{
			clientobj[clientFd].ResponseChunked = 1;
			clients[clientFd].response = Response::buildResponse(403, "Forbidden",config[i].ErrorPages[403], clientFd, clientobj, _configs);
		}
    }
	else
	{
		clientobj[clientFd].ResponseChunked = 1;
		clients[clientFd].response = Response::buildResponse(404, "Not Found",config[i].ErrorPages[404], clientFd, clientobj, _configs);
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
    if (!CheckMethodeIsAllowed("GET", _configs, conf_i, key))
    {
        clients[clientFd].response = Response::buildResponse(403, "Forbidden",_configs[conf_i].ErrorPages[403], clientFd, clientobj, _configs);
        return;
    }
    if (_configs[conf_i].locations[key].Return.empty())
		CheckDirOrFile(value, clientFd, _configs, conf_i, key, clientobj, epoll);
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
			clients[clientFd].response = Response::buildResponse(404, "Not Found",config.ErrorPages[404], clientFd, clientobj, _configs);
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
        clients[clientFd].response = Response::buildResponse(404, "Not Found",config.ErrorPages[404], clientFd, clientobj, _configs);
		return ;
	}
	if(S_ISDIR(st.st_mode))
	{
		if (fullpath[fullpath.size() - 1] != '/')
		{
        	clients[clientFd].response = Response::buildResponse(409, "conflict",config.ErrorPages[409], clientFd, clientobj, _configs);
			return ;
		}
		if(!is_directory_empty(fullpath))
		{
			if (!delete_dir_recursive(fullpath, clientFd, config, r, clientobj))
			{
				clients[clientFd].response = Response::buildResponse(500, "Internal Server Error",config.ErrorPages[500], clientFd, clientobj, _configs);
				return;
			}
			clients[clientFd].response= clients[clientFd].response.buildResponse(204, "No Content",config.ErrorPages[204], clientFd, clientobj, _configs);
			return ;
		}
		if(remove(fullpath.c_str()) == 0)
		{
			clients[clientFd].response= clients[clientFd].response.buildResponse(204, "No Content",config.ErrorPages[204], clientFd, clientobj, _configs);
			return ;
		}
		else  // Failed to remove directory
		{
			clients[clientFd].response= clients[clientFd].response.buildResponse(500, "Internal Server Error",config.ErrorPages[500], clientFd, clientobj, _configs);
			return ;
		}
	}
	else if (S_ISREG(st.st_mode))
	{
		if(access(fullpath.c_str(), W_OK) != 0)  // doesn't have permission for delete
		{
			clients[clientFd].response= clients[clientFd].response.buildResponse(403, "Forbidden",config.ErrorPages[403], clientFd, clientobj, _configs);
			return;
		}
		if (std::remove(fullpath.c_str()) == 0)
		{
			clients[clientFd].response= clients[clientFd].response.buildResponse(204, "No Content",config.ErrorPages[204], clientFd, clientobj, _configs);
			std::cout << "✅ File deleted: " << fullpath << std::endl;
			return;
		}
		else
		{
			clients[clientFd].response= clients[clientFd].response.buildResponse(500, "Internal Server Error",config.ErrorPages[500], clientFd, clientobj, _configs);
			return;
		}
	}
	else //Unsupported file type (e.g., socket, symlink)
	{
		clients[clientFd].response= clients[clientFd].response.buildResponse(403, "Forbidden",config.ErrorPages[403], clientFd, clientobj, _configs);
		return;
	}
}

std::string generateId1(size_t length)
{
    const char set[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    std::string result;
    result.reserve(length);

    for (size_t i = 0; i < length; i++)
        result += set[rand() % (sizeof(set) - 1)];
    return result;
}

std::string abstract_file(std::string fullpath)
{
	size_t pos = 0;
	std::string save = fullpath;
	pos = fullpath.find('/');
	while(!fullpath.empty() && pos != std::string::npos)
	{
		if(pos != std::string::npos)
		{
			fullpath = fullpath.substr(pos+1);
			pos = fullpath.find('/');
		}
	}
	return fullpath;
}


void Server::handle_delete_methode(request r, std::vector<ServerConfig> _configs, int clientFd, size_t conf_i, std::map<int, Client>& clientobj)
{
	std::map<int, std::string> map;
    map = getMatchingRootPath(r, _configs[conf_i]);
	int key = map.begin()->first;
	if (!_configs[clientobj[clientFd].conf_i].locations[key].Return.empty())
	{
		clientobj[clientFd].statusCode = _configs[clientobj[clientFd].conf_i].locations[key].Return.begin()->first;
		clientobj[clientFd].statusMsg = "Found";
		clientobj[clientFd].ReturnLocation = _configs[clientobj[clientFd].conf_i].locations[key].Return.begin()->second;
        clientobj[clientFd].body_complete = 1;
		std::cout << "innnn handle return in delete: " << _configs[clientobj[clientFd].conf_i].locations[key].Return.begin()->first << " \n";
        return ;
	}
	std::string fullpath = mergePaths(_configs[clients[clientFd].conf_i].locations[map.begin()->first].root, _configs[clients[clientFd].conf_i].locations[map.begin()->first].upload_store);
	//std::cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!FULLPATH 11111 "<< fullpath <<std::endl;
    fullpath = mergePaths(fullpath, r.get_path());
	//std::cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!FULLPATH 2222 "<< fullpath <<std::endl;
    if (!CheckMethodeIsAllowed("DELETE", _configs, conf_i, key))
    {
		clients[clientFd].response= clients[clientFd].response.buildResponse(403, "Forbidden", _configs[conf_i].ErrorPages[403], clientFd, clientobj, _configs);
        return;
    }
	if (r.get_path()[r.get_path().size() - 1] == '/')
		fullpath += '/';
	std::cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!FULLPATH"<< fullpath <<std::endl;
    dir_or_file(fullpath, clientFd, _configs[conf_i], r, clientobj); 
}
bool error_post( std::map<int, Client> &clients,int clientFd, std::vector<ServerConfig> _configs,request & r, size_t conf_i)
{
	std::map<int, std::string> map;
    map = getMatchingRootPath(r, _configs[conf_i]);
	int key = map.begin()->first;
	if (!CheckMethodeIsAllowed("POST", _configs, conf_i, key))
    {
		clients[clientFd].response= Response::buildResponse(403, "Forbidden",_configs[clients[clientFd].conf_i].ErrorPages[403], clientFd, clients, _configs);
        return 0;
    }

	if (clients[clientFd].PostBody.size() >static_cast<size_t>( _configs[clients[clientFd].conf_i].client_max_body_size ))
	{
		clients[clientFd].response= Response::buildResponse(413, "Payload Too Large",_configs[clients[clientFd].conf_i].ErrorPages[413], clientFd, clients, _configs);
		return 0;
	}
	if (_configs[clients[clientFd].conf_i].locations[map.begin()->first].upload_store.empty())
	{
		clients[clientFd].response= Response::buildResponse(403, "Forbidden",_configs[clients[clientFd].conf_i].ErrorPages[403], clientFd, clients, _configs);
		return 0;
	}
	// if (!_configs[conf_i].locations[key].Return.empty())
	// {
	// 	clientobj[clientFd].statusCode = _configs[conf_i].locations[key].Return.begin()->first;
	// 	clientobj[clientFd].statusMsg = "Found";
	// 	clientobj[clientFd].ReturnLocation = _configs[conf_i].locations[key].Return.begin()->second;
	// 	return;
	// }
	return 1;
}

std::map<std::string,std::string>  parce_header_cgi(std::string &header)
{
	std::map<std::string , std::string>map_h;
		size_t i ;
		while((i = header.find(":")) != std::string::npos)
		{
			std::string key = header.substr(0, i);
			header = header.substr(i + 1);
			key = trim1(key);
			i = header.find('\n');
			std::string value = header.substr(0, i + 1);
			value = trim1(value);
			size_t i1 = value.find(";");
			value = value.substr(0,i1);
			std::cout << "key^^^^^^^^^^^^^"<< key << std::endl;
			map_h[key] = value;
			std::cout << "value^^^^^^^^^^^^^"<< value << std::endl;
			header = header.substr(i +1);
		}
		return map_h;
}


int get_code_status_cgi(std::map<std::string, std::string> map_h)
{
	std::map<std::string, std::string>::iterator it2 = map_h.find("Status");
	int status_code = 201;
	if (it2 != map_h.end())
	{
			std::string code_cgi = it2->second;
			std::istringstream iss(code_cgi);
			iss >> status_code;
			if (iss.fail()) 
			{
				status_code = 500;
				std::cerr << "⚠️ Impossible de parser Status: " << code_cgi << std::endl;
			} 
			else {
					if (status_code < 100 ||  status_code > 599)
							status_code = 500;
					else
						std::cout << "✅ Code CGI extrait = [" << status_code<< "]" << std::endl;
			}
	}
	else
		std::cerr << "❌ Aucun header Status trouvé dans CGI" << std::endl;
	return status_code;
}

bool handel_cgi_post(std::vector<ServerConfig> _configs, int clientFd, std::map<int, Client> &clientobj)
{
	struct stat statbuf;
	if(clientobj[clientFd].cgiMap[clientFd].signal || clientobj[clientFd].cgiMap[clientFd].Timeout)
	{
		if(remove(clientobj[clientFd].filename.c_str()))
			return 0;	
		return 0;
	}
	if(clientobj[clientFd].cgiMap[clientFd].exit_code_cgi != 0)
	{
		clientobj[clientFd].response= Response::buildResponse(502, "Bad Gateway",_configs[clientobj[clientFd].conf_i].ErrorPages[502], clientFd, clientobj, _configs);
		if(remove(clientobj[clientFd].filename.c_str()))
			return 0;
		return 0;
	}
	std::ofstream out(clientobj[clientFd].filename.c_str(),std::ios::binary | std::ios::trunc);
	if(!out)
	{
			if(stat(clientobj[clientFd].filename.c_str(), &statbuf) == -1)
				clientobj[clientFd].response= Response::buildResponse(500, "Internal Server Error",_configs[clientobj[clientFd].conf_i].ErrorPages[500], clientFd, clientobj, _configs);
			else if(access(clientobj[clientFd].filename.c_str(), W_OK) == -1)
				clientobj[clientFd].response= Response::buildResponse(403, "Forbiden",_configs[clientobj[clientFd].conf_i].ErrorPages[403], clientFd, clientobj, _configs);
			else
				clientobj[clientFd].response = Response::buildResponse(500, "Internal Server Error", _configs[clientobj[clientFd].conf_i].ErrorPages[500],clientFd, clientobj, _configs);
			std::cerr << "❌ Failed to open file: " << clientobj[clientFd].filename << std::endl;
			out.close();
			if(remove(clientobj[clientFd].filename.c_str()))
				return 0;
			return 0;
	}
		size_t header_end  = 0;
		std::string header = "";
		if((header_end = clientobj[clientFd].CgiBody.find("\r\n\r\n")) != std::string::npos)
		{
			header = clientobj[clientFd].CgiBody.substr(0, header_end + 4);
			clientobj[clientFd].CgiBody = clientobj[clientFd].CgiBody.substr(header.size());
		}
		else if ((header_end = clientobj[clientFd].CgiBody.find("\n\n")) != std::string::npos )
		{
			header = clientobj[clientFd].CgiBody.substr(0, header_end + 2);
			clientobj[clientFd].CgiBody = clientobj[clientFd].CgiBody.substr(header.size());
		}
		std::map<std::string, std::string> map_h = parce_header_cgi(header);
		
		std::string save = clientobj[clientFd].filename;
		size_t i2 = clientobj[clientFd].filename.find(".");
		std::map<std::string , std::string>::iterator a = map_h.find("Content-Type");
		std::string str = "";
		size_t b = 0;
		if(a != map_h.end())
		{
			str = a->second;
			b = str.find("/");
			str = str.substr(b+1);
			clientobj[clientFd].filename = clientobj[clientFd].filename.substr(0,i2+1) + str;
		}
	else
		clientobj[clientFd].filename = clientobj[clientFd].filename.substr(0,i2+1) + "plain";
	int status_code = get_code_status_cgi(map_h);
	std::map<std::string, std::string>::iterator it = map_h.find("Content-Length");
	if (it != map_h.end())
	{
		std::string contentLength = it->second;
		double siz = std::atof(contentLength.c_str());
		out.write(clientobj[clientFd].CgiBody.c_str(), siz);
		std::cout << "Content-Length found: " << contentLength << std::endl;
	}
	else
		out.write(clientobj[clientFd].CgiBody.c_str(), clientobj[clientFd].CgiBody.size());
	out.flush();
	std::cout<< " file is " <<clientobj[clientFd].filename << std::endl;;
	if (std::rename(save.c_str(), clientobj[clientFd].filename.c_str()) == 0)
		std::cout << "File renamed successfully!\n";
	else 
	{
		std::cerr << "❌ Failed to rename file: " << clientobj[clientFd].filename << std::endl;
		clientobj[clientFd].response= Response::buildResponse(500, "Internal Server Error",_configs[clientobj[clientFd].conf_i].ErrorPages[500], clientFd, clientobj, _configs);
		out.close();
		if(remove(clientobj[clientFd].filename.c_str()))
			return 0;
		return 0;
	}
	clientobj[clientFd].response= Response::buildResponse(status_code, "Created",_configs[clientobj[clientFd].conf_i].ErrorPages[status_code], clientFd, clientobj, _configs);
	out.close();
	return 0;
}

void Server::handle_post_methode(request & r, std::vector<ServerConfig> _configs, int clientFd, size_t conf_i, std::map<int, Client> &clientobj,EpollManager &epollManager)
{
	
	std::map<int, std::string> map;
	map = getMatchingRootPath(r, _configs[conf_i]);
	int key = map.begin()->first;
    if (!_configs[clientobj[clientFd].conf_i].locations[key].Return.empty())
	{
		clientobj[clientFd].statusCode = _configs[clientobj[clientFd].conf_i].locations[key].Return.begin()->first;
		clientobj[clientFd].statusMsg = "Found";
		clientobj[clientFd].ReturnLocation = _configs[clientobj[clientFd].conf_i].locations[key].Return.begin()->second;
        clientobj[clientFd].body_complete = 1;
        return ;
	}
	if (clientobj[clientFd].has_cgi)
	{
		if(!handel_cgi_post(_configs, clientFd,  clientobj))
		return;
	}
	if(!error_post( clients,clientFd,  _configs,r, conf_i))
		return ;
	std::cout << " in post%%%%%%%%%\n <<<<<<<<<<<< "<< clientobj[clientFd].path<< std::endl;;
	struct stat statbuf;
	std::ostringstream filename;
    std::string fullpath = map.begin()->second;
	std::string abstract_fil =  abstract_file(clientobj[clientFd].path);
	size_t b = abstract_fil.find_last_of('.');
	std::string extt = "";
	std::string inter = "";
    if (b == std::string::npos || b == abstract_fil.size() - 1) {
		extt = "";
    }
	else 
		extt = abstract_fil.substr(b);
	if(!abstract_fil.empty())
	{
		size_t e = fullpath.find(abstract_fil);
		if(e != std::string::npos)
			 fullpath = fullpath.substr(0,e);
	 inter = find_matching_inter(extt, _configs, conf_i , key);
	std::cout << " matchinggggggggggg interprettttttttt ::::  "<< abstract_fil << "  "<< extt << std::endl;
	}
	fullpath = mergePaths(_configs[clients[clientFd].conf_i].locations[map.begin()->first].root, _configs[clients[clientFd].conf_i].locations[map.begin()->first].upload_store);
	std::cout << "join ****************hiiiiiiiuploading path for post is:" << fullpath << std::endl;

	if(inter.empty())
	{
		std::cout << " no cgiiiiiiiiiiiiiiiiiiiiiiiin sizz de bodyyyyyyyyyyy << " <<clientobj[clientFd].PostBody.size()  <<std::endl;; 
		std::map<std::string, std::string> header_ = clientobj[clientFd].map;
		std::map<std::string, std::string>::iterator it_ = header_.find("Content-Type");
		std::string type = "";
		if(it_ != header_.end())
		{
			size_t ext = clientobj[clientFd].ContentType.find('/');
			type = clientobj[clientFd].ContentType.substr(ext + 1);
		}
		else
			type = "plain";
		srand(time(NULL));
		filename << fullpath << "/" << generateId1() << "." << type  ;
		std::cout << "\nfile is uploaded in: " << filename.str() << std::endl;
		std::ofstream out(filename.str().c_str(),std::ios::binary);
		if(!out)
		{
			if(stat(clientobj[clientFd].filename.c_str(), &statbuf) == -1)
				clientobj[clientFd].response= Response::buildResponse(500, "Internal Server Error",_configs[clientobj[clientFd].conf_i].ErrorPages[500], clientFd, clientobj, _configs);
			else if(access(clientobj[clientFd].filename.c_str(), W_OK) == -1)
				clientobj[clientFd].response= Response::buildResponse(403, "Forbiden",_configs[clientobj[clientFd].conf_i].ErrorPages[403], clientFd, clientobj, _configs);
			else
				clientobj[clientFd].response = Response::buildResponse(500, "Internal Server Error", _configs[clientobj[clientFd].conf_i].ErrorPages[500],clientFd, clientobj, _configs);
			std::cerr << "❌ Failed to open file: " << clientobj[clientFd].filename << std::endl;
			out.close();
			remove(clientobj[clientFd].filename.c_str());
			return;
		}
		if(clientobj[clientFd].chnked ==1 && clientobj[clientFd].body_complete == 1)
			out.write(clientobj[clientFd].body_chunked.c_str(), clientobj[clientFd].body_chunked.size());
		else 
			out.write(clientobj[clientFd].PostBody.c_str(), clientobj[clientFd].PostBody.size());
		out.flush();
		out.close();
	}
	else if(!inter.empty() )
	{
		srand(time(NULL));
		filename << fullpath << "/" << generateId1()  << extt;
		clientobj[clientFd].filename = filename.str();
		std::ofstream out(filename.str().c_str(),std::ios::binary);
		if(!out)
		{
			if(stat(clientobj[clientFd].filename.c_str(), &statbuf) == -1)
				clientobj[clientFd].response= Response::buildResponse(500, "Internal Server Error",_configs[clientobj[clientFd].conf_i].ErrorPages[500], clientFd, clientobj, _configs);
			else if(access(clientobj[clientFd].filename.c_str(), W_OK) == -1)
				clientobj[clientFd].response= Response::buildResponse(403, "Forbiden",_configs[clientobj[clientFd].conf_i].ErrorPages[403], clientFd, clientobj, _configs);
			else
				clientobj[clientFd].response = Response::buildResponse(500, "Internal Server Error", _configs[clientobj[clientFd].conf_i].ErrorPages[500],clientFd, clientobj, _configs);
			std::cerr << "❌ Failed to open file: " << clientobj[clientFd].filename << std::endl;
			out.close();
			remove(clientobj[clientFd].filename.c_str());
			return;
		}
		if(clientobj[clientFd].chnked ==1 && clientobj[clientFd].body_complete == 1)
		{
			out.write(clientobj[clientFd].body_chunked.c_str(), clientobj[clientFd].body_chunked.size());
		}
		else 
			out.write(clientobj[clientFd].PostBody.c_str(), clientobj[clientFd].PostBody.size());
		execute_cgi(clientFd,clientobj,filename.str(), inter,epollManager);
		out.flush();
		out.close();
	}
	if (!clientobj[clientFd].has_cgi)
		clients[clientFd].response= Response::buildResponse(201, "Created",_configs[clients[clientFd].conf_i].ErrorPages[201], clientFd, clientobj, _configs);

}







