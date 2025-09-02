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

	// Remove trailing '/' from root if present
	if (!fixedRoot.empty() && fixedRoot[fixedRoot.length() - 1] == '/')
		fixedRoot.erase(fixedRoot.length() - 1);

	// Remove leading '/' from suffix if present
	if (!fixedSuffix.empty() && fixedSuffix[0] == '/')
		fixedSuffix = fixedSuffix.substr(1);

	// Concatenate with a single '/'
	return fixedRoot + "/" + fixedSuffix;
}



std::map<int, std::string> getMatchingRootPath(request &r, ServerConfig &config)
{
	std::string requestedPath = r.get_path(); // e.g. "/index.html"
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
		locPath = config.locations[i].path;
		Pchunks = pathchunks(locPath);
		if (Pchunks.size() == 1)
			locPath = Pchunks[0];
		else
			locPath = Pchunks[1];
		if (locPath[1] == '\0' && requestedPath.empty())
		{
			result[i] = config.locations[i].root;
			return result;
		}
		if (Pchunks.size() == 2 && Pchunks[0] == "^~")
		{
			if (requestedPath.find(locPath) == 0)
			{
				matchedRoot = join_path(config.locations[i].root, requestedPath.substr(locPath.length()));
				result[i] = matchedRoot;
				return result;
			}
		}
		else if (requestedPath.find(locPath) == 0 && locPath.length() > maxMatchLength)
		{
			maxMatchLength = locPath.length();
			matchedRoot = join_path(config.locations[i].root, requestedPath.substr(locPath.length()));
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
			{
				std::cout << "method in it "<< *it<< std::endl;
				return 1;
			}
			++it;
		}
	}
	return 0;
}


std::string send_dir_list(int clientFd, std::string requested_path)
{
	std::stringstream body;
	DIR* directory = opendir(requested_path.c_str());
	struct dirent *entry;
	if (!directory)
	{
		body << "<html><body><h1>Unable to open director" << requested_path << "</h1></body></html>";
		return "Couldn't open dir!";
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
	// body << "<html><body><h1>Hello Atoindex Is On !</h1></body></html>";
	std::ostringstream response;
	response << "HTTP/1.1 200 OK\r\n";
	response << "Content-Length:" << body.str().size() << "\r\n";
	response << "Content-Type: text/html\r\n";
	response << "Connection: close\r\n";
	response << "\r\n";
	response << body.str();
	ssize_t sent = send(clientFd, response.str().c_str(), response.str().size(), 0);
	if (sent < 0)
		std::cerr << "Something went wrong send < 0" << std::endl;
	else
	{
		std::cout << "✅ Response being send to FD: " << clientFd << std::endl;
		return "autoindex:Dooone ✅";
	}
    return ("SomeThing went wrong");
}


std::string Server:: CheckDirOrFile(std::string requested_path, int clientFd, std::vector<ServerConfig> config, int i, int key, request &r, std::map<int, Client> clientobj)
{
	struct stat statbuf;
    if (stat(requested_path.c_str(), &statbuf) == 0)
	{
        if (S_ISREG(statbuf.st_mode))//Check is a valid file then serve it
		{
			std::cout << "$$$$$$$$$$$$$$the path is: " << requested_path << std::endl;
			 clients[clientFd].response  = clients[clientFd].response.buildResponse(r, 200, "OK", requested_path, clientFd, clientobj);
        }
		else if (S_ISDIR(statbuf.st_mode)) // if it is a dir attache the index file then serve it
		{
			std::string index_file;
			index_file = join_path(requested_path, config[i].locations[key].index);

            if (stat(index_file.c_str(), &statbuf) == 0 && S_ISREG(statbuf.st_mode)) // file found ->means everything is good
			 	clients[clientFd].response= clients[clientFd].response.buildResponse(r, 200, "OK", index_file, clientFd, clientobj);
			else if (config[i].locations[key].autoindex)// Not found pass to autoindex result
			{
				return send_dir_list(clientFd, requested_path);// using requested path only !
				// change return value to void and make buildresponse instead of send
            }
			else
				clients[clientFd].response = Response::buildResponse(r, 403, "Forbidden",config[i].ErrorPages[403], clientFd, clientobj);
        }
		else // If we did attach the file but still it's not found
			clients[clientFd].response = Response::buildResponse(r, 403, "Forbidden",config[i].ErrorPages[403], clientFd, clientobj);
    }
	else
		clients[clientFd].response = Response::buildResponse(r, 404, "Not Found",config[i].ErrorPages[404], clientFd, clientobj);
	return NULL;
}

void Server::handle_get_methode(request &r, std::vector<ServerConfig> _configs, int clientFd, size_t conf_i, std::map<int, Client> clientobj)
{
	std::map<int, std::string> map;
    map = getMatchingRootPath(r, _configs[conf_i]);
    int key = map.begin()->first;
    std::string value = map.begin()->second;
    if (!CheckMethodeIsAllowed("GET", _configs, conf_i, key))
    {
        //RequestResponse(clientFd, _configs[conf_i].ErrorPages[405], "405 Method Not Allowed");
        clients[clientFd].response = Response::buildResponse(r, 405, "Method Not Allowed",_configs[conf_i].ErrorPages[405], clientFd, clientobj);
        return;
    }
    std::cout << "\nFull path is:" << value << std::endl;
    std::string ret = CheckDirOrFile(value, clientFd, _configs, conf_i, key, r, clientobj);
    std::cout << ret << std::endl;
}


// void send_response(int clientFd, int status_code, const std::string &status_text, const std::string &body)
// {
//     std::ostringstream response;
//     response << "HTTP/1.1 " << status_code << " " << status_text << "\r\n";
//     if (!body.empty())
//     {
//         response << "Content-Type: text/html\r\n";
//         response << "Content-Length: " << body.size() << "\r\n";
//     }
//     response << "Connection: close\r\n";
//     response << "\r\n";
//     response << body;
// 	// std::cout << "SALAM BEAUTIFULLLL WORLD HERE   " << std::endl;

//     std::string resp_str = response.str();
//     ssize_t sent = send(clientFd, resp_str.c_str(), resp_str.size(), 0);
//     if (sent < 0)
//         std::cerr << "❌ send failed: " << strerror(errno) << std::endl;
//     else
//     {
// 		std::cout << "Response sent to FD: " << clientFd << std::endl;
// 	}
// 	//std::cout << "AND HEREEE TOOO\n";
// }

// void send_newresponse(int clientFd, int status_code, const std::string &status_text, const std::string &body, std::string type)
// {
//     std::ostringstream response;
//     response << "HTTP/1.1 " << status_code << " " << status_text << "\r\n";
//     if (!body.empty())
//     {
//         response << "Content-Type: " << type << "\r\n";
//         response << "Content-Length: " << body.size() << "\r\n";
//     }
//     response << "Connection: close\r\n";
//     response << "\r\n";
//     response << body;

//     std::string resp_str = response.str();
//     ssize_t sent = send(clientFd, resp_str.c_str(), resp_str.size(), 0);
//     if (sent < 0)
//         std::cerr << "❌ send failed: " << strerror(errno) << std::endl;
//     else
//         std::cout << "Response sent to FD: " << clientFd << std::endl;
// }


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
		//send_response(clientFd, 404, "Not Found", load_html_file("www/404.html"));
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

void Server::handle_delete_methode(request r, std::vector<ServerConfig> _configs, int clientFd, size_t conf_i, std::map<int, Client> clientobj)
{
	// if (r.get_path().find("..") != std::string::npos) //If you do not block paths containing .., an attacker might delete or access files outside the allowed web root directory, leading to security vulnerabilities.
	// {
	// 	send_response(clientFd, 400, "Bad Request", "<html><body><h1>400 Bad Request</h1><p>Invalid path.</p></body></html>");
	// 	return;
	// }
	std::map<int, std::string> map;
    map = getMatchingRootPath(r, _configs[conf_i]);
    int key = map.begin()->first;
    if (!CheckMethodeIsAllowed("DELETE", _configs, conf_i, key))
    {
        //send_response(clientFd, 405, "Method Not Allowed", load_html_file("www/405.html"));
		clients[clientFd].response= clients[clientFd].response.buildResponse(r, 405, "Method Not Allowed", _configs[conf_i].ErrorPages[405], clientFd, clientobj);
        return;
    }
    std::string fullpath = map.begin()->second;
    std::cout << "\nFull path is:" << fullpath << std::endl;
    dir_or_file(fullpath, clientFd, _configs[conf_i], r, clientobj);
}


// void send_repons_post(int clientFd, const Response& repo)
// {
// 	std::ostringstream response;
// 	response << "HTTP/1.1 " << repo.reponse_status << " OK\r\n";
// 	response << "Content-Length: " << repo.response_body.size() << "\r\n";
// 	response << "Content-Type: text/plain\r\n";
// 	response << "\r\n";
// 	response << repo.response_body;
// 	send(clientFd, response.str().c_str(), response.str().size(), 0);
// }

void Server::handle_post_methode(request & r, std::vector<ServerConfig> _configs, int clientFd, size_t conf_i, std::map<int, Client> clientobj)
{
	std::map<int, std::string> map;
        std::cout<<"________________________"<<r.ContentType<<"\n\n";
	
    map = getMatchingRootPath(r, _configs[conf_i]);
		std::cout << "&&&&&&&&in return here \n\n";
    if (!CheckMethodeIsAllowed("POST", _configs, conf_i, map.begin()->first))
    {
		//std::cout << "here internalllll\n";
        //send_response(clientFd, 405, "Method Not Allowed", load_html_file("www/405.html"));
		clients[clientFd].response= clients[clientFd].response.buildResponse(r, 405, "Method Not Allowed",_configs[conf_i].ErrorPages[405], clientFd, clientobj);
        return;
    }
	if (_configs[conf_i].locations[map.begin()->first].upload_store.empty())
	{
		//std::cout << "here internalllll\n";
		//send_response(clientFd, 500, "Internal Server Error", load_html_file("www/500.html"));
		clients[clientFd].response= clients[clientFd].response.buildResponse(r, 500, "Internal Server Error",_configs[conf_i].ErrorPages[500], clientFd, clientobj);
		return ;
	}
    std::string fullpath = map.begin()->second;
	fullpath = join_path(fullpath, _configs[conf_i].locations[map.begin()->first].upload_store);
    std::cout << "\nFull path is:" << fullpath << std::endl;
	
	if ((long)r.body.size() > _configs[conf_i].client_max_body_size)
	{
		std::cout << "here internalllll\n";
		//send_response(clientFd, 413, "Payload Too Large", load_html_file("www/413.html"));
		clients[clientFd].response= clients[clientFd].response.buildResponse(r, 413, "Payload Too Large",_configs[conf_i].ErrorPages[413], clientFd, clientobj);
		return ;
	}
	// std::ostringstream filename;
	// std::cout << "^^^^^" << r.ContentType << std::endl;
	// int ext = r.ContentType.find('/');
	// r.ContentType = r.ContentType.substr(ext + 1);
	// filename << fullpath << "/" << rand() <<"."<<r.ContentType;
	// std::cout << "****" << filename.str() << std::endl;
	// std::ofstream out(filename.str().c_str(),std::ios::binary);
	// if(!out)
	// {
	// 	std::cerr << "❌ Failed to open file: " << filename.str() << std::endl;
	// 	send_newresponse(clientFd, 500, "Internal Server Error", load_html_file("www/500.html"), r.ContentType);
	// 	return;
	// }
	// // std::cout << "❌❌❌❌❌❌❌❌❌❌body--------" << r.body << "-----------" <<"\n\n";
	// //out << r.body;
	// //std::cout << "the body is; " << r.body.c_str();

	// std::cout << "✅✅✅✅✅✅✅✅✅✅" << r.body << std::endl;
	// out .write(r.body.c_str(), r.body.size());
	// out.flush();
	// out.close();
	//send_response(clientFd, 201, "Created", load_html_file("www/201.html"));
	clients[clientFd].response= clients[clientFd].response.buildResponse(r, 201, "Created",_configs[conf_i].ErrorPages[201], clientFd, clientobj);
     std::cout<< "❌❌❌❌❌❌❌❌❌❌body "<<clients[clientFd].response.body   << " content type :    "  << clients[clientFd].response.contentType << "  code:  "<< clients[clientFd].response.statusCode << " msg :" << clients[clientFd].response.statusMsg << "\n\n";
	// else
	// {
    // 	 send_response(clientFd, 500, "Internal Server Error", load_html_file("www/500.html"));
	// 	return ;
	// }
}

// void handle_post_methode(request & r, std::vector<ServerConfig> _configs, int clientFd, int port)
// {
// 	std::map<int, std::string> map;
// 	reponse repo;
// 	for (size_t i = 0; i < _configs.size(); ++i)
// 	{
// 		if (_configs[i].port == port)
// 		{
// 			map = getMatchingRootPath(r, _configs[i]);
// 			if(r.get_path() == "/upload")
// 			{
// 				std::map<int, std::string> fullPath_map = getMatchingRootPath(r, _configs[i]);
// 				std::map<int, std::string>::iterator it = fullPath_map.begin();
// 				if (!CheckMethodeIsAllowed("POST", _configs, i, it->first))
// 				{
// 						repo.reponse_status = 405 ;
// 						repo.response_body =" Method Not Allowed.";
// 						send_repons_post(clientFd, repo);
// 						return ;
// 				}
// 				else if ((long)r.body.size() > _configs[i].client_max_body_size)
// 				{
// 						repo.reponse_status = 413;
// 						repo.response_body =" Payload Too Large.";
// 						send_repons_post(clientFd, repo);
// 						return ;
// 				}
// 				std::ostringstream filename;
// 			 	filename << it->second<< "/upload_" << std::time(0) << ".txt";;
// 				std::ofstream out(filename.str().c_str(),std::ios::binary);
// 				if(!out)
// 				{
// 					std::cerr << "❌ Failed to open file: " << filename.str() << std::endl;
// 					repo.reponse_status = 500;
// 					repo.response_body = "Internal Server Error";
// 					return;
// 				}
// 				out << r.body;
// 				out.close();
// 				repo.reponse_status = 201;
// 				repo.response_body = "File uploaded successfully!\n";
// 				send_repons_post(clientFd, repo);
// 			}
// 			return;
// 		}
// }
// }