/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   methods.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: wzahir <wzahir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/28 15:57:20 by wzahir            #+#    #+#             */
/*   Updated: 2025/08/23 22:00:14 by wzahir           ###   ########.fr       */
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

// std::cout << "----" << locPath << "----" << std::endl;
// exit (00);

// std::cout << "req is: -" << requestedPath << "--" << std::endl;
// std::cout << "loc is: -" << locPath << "--" << std::endl;


// std::cout << "The request path is: " << requestedPath << "\nAnd the corr.loc path is: " << locPath << std::endl;
// std::cout << "--- and plus the rootifound is: " << matchedRoot << std::endl;


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
	//std::cout << "************" << requestedPath << std::endl;

	std::map<int, std::string> result;

	if (!getFileExtension(requestedPath).empty())
	{
		extension = getFileExtension(requestedPath);
		// std::cout << "The request extension is: *_*" << extension << "*_*" << std::endl;
		while (i < config.locations.size())
		{
			locPath = config.locations[i].path;
			Pchunks = pathchunks(locPath);
			if (Pchunks.size() == 2 && Pchunks[0] == "~")
			{
				// std::cout << "your server location file extensions are: -" << Pchunks[1].substr(2) << "--" << std::endl;
				if (Pchunks[1].substr(2) == extension)
				{
					// std::cout << "\n -->" << requestedPath << std::endl << std::endl;
					matchedRoot = config.locations[i].root + requestedPath;
					result[i] = matchedRoot;
					return result;
				}
			}
			i++;
		}
		// now search for the ~ in locations 
		// and extract the extension 
		// then see if there is some one that matches 
		// if found make str matchedRoot
		// That's itttt Let's Gooooo.
	}
	i = 0;
	while (i < config.locations.size())
	{
		locPath = config.locations[i].path;
		Pchunks = pathchunks(locPath);
		if (Pchunks.size() == 1)
		{
			locPath = Pchunks[0];
		//	std::cout << "And the locpath is: " << locPath << "----- and it's len is: " << locPath.length() << std::endl;
		}
		else
		{
			locPath = Pchunks[1];
		}
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
				// std::cout << "resutl in find matching is: " << matchedRoot << std::endl;
				return result;
			}
		}
		else if (requestedPath.find(locPath) == 0 && locPath.length() > maxMatchLength)
		{
			//std::cout << "CODE GOT TILL HERE hh but dono what to do\n" << std::endl;
			maxMatchLength = locPath.length();
			matchedRoot = join_path(config.locations[i].root, requestedPath.substr(locPath.length()));
			coorLoc = i;
			// std::cout << "root is: -" << config.locations[i].root << "- and req root is: -" << requestedPath.substr(locPath.length()) << "--\n";
		}
		i++;
	}
	// matchedRoot += "/"; ///////////to handle this because without it get methode works with it delete works
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


std::string CheckDirOrFile(std::string requested_path, int clientFd, std::vector<ServerConfig> config, int i, int key)
{
	struct stat statbuf;
	// std::cout << "\n\n Your fileis :" << requested_path << "\n\n";
    if (stat(requested_path.c_str(), &statbuf) == 0)
	{
        if (S_ISREG(statbuf.st_mode))//Check is a valid file then serve it
		{
			return RequestResponse(clientFd, requested_path, "200 OK");
        }
		else if (S_ISDIR(statbuf.st_mode)) // if it is a dir attache the index file then serve it
		{
			std::string index_file;
			index_file = join_path(requested_path, config[i].locations[key].index);

			// std::cout << "the index is: " << index_file << "\n\n";
            if (stat(index_file.c_str(), &statbuf) == 0 && S_ISREG(statbuf.st_mode)) // file found ->means everything is good
			{
                return RequestResponse(clientFd, index_file, "200 OK");
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


void handle_get_methode(request r, std::vector<ServerConfig> _configs, int clientFd, size_t conf_i)
{
	std::map<int, std::string> map;
	std::map<std::string, std::string> my_map = r.get_header();
	for (auto i : my_map)
	{
		std::cout << " the header is: " << i.first << ": " << i.second << std::endl;
	}
	std::cout << "hello I am trying to know some data here:\n";
	std::cout << " the body is: " << r.get_body() << std::endl;
	std::cout << " and it's len is going to be: " << r.get_body().size() << std::endl;
    map = getMatchingRootPath(r, _configs[conf_i]);
    int key = map.begin()->first;
    std::string value = map.begin()->second;
    std::cout << "the path in appropriate location in app. server->" << _configs[conf_i].locations[key].path << std::endl;
    // if (isKey(_configs[i].locations[key].path, "/cgi-bin") && CheckMethodeIsAllowed("GET", _configs, i, key))
    // {
    // 	std::cout << "salam it's a cgi hereee\n";
    // 	std::string ret = CheckDirOrFileCGI(value, clientFd, _configs, i, key, r);
    // 	std::cout << ret << std::endl;
    // 	return ;
    // }
    if (!CheckMethodeIsAllowed("GET", _configs, conf_i, key))
    {
        std::cout << "This means method not allowed \n\n" << std::endl;
        RequestResponse(clientFd, _configs[conf_i].ErrorPages[405], "405 Method Not Allowed");
        return;
    }
    std::cout << "Yaaay method found this means method allowed\n\n" << std::endl;
    std::cout << "\nFull path is:" << value << std::endl;
    std::string ret = CheckDirOrFile(value, clientFd, _configs, conf_i, key);
    std::cout << ret << std::endl;
}


void send_response(int clientFd, int status_code, const std::string &status_text, const std::string &body)
{
    std::ostringstream response;
    response << "HTTP/1.1 " << status_code << " " << status_text << "\r\n";
    if (!body.empty())
    {
        response << "Content-Type: text/html\r\n";
        response << "Content-Length: " << body.size() << "\r\n";
    }
    response << "Connection: close\r\n";
    response << "\r\n";
    response << body;

    std::string resp_str = response.str();
    ssize_t sent = send(clientFd, resp_str.c_str(), resp_str.size(), 0);
    if (sent < 0)
        std::cerr << "❌ send failed: " << strerror(errno) << std::endl;
    else
        std::cout << "Response sent to FD: " << clientFd << std::endl;
}

bool delete_dir_recursive(std::string &path, int clientFd)
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
				delete_dir_recursive(fullpath, clientFd);
			else
				remove(fullpath.c_str());
		}
		else
			send_response(clientFd, 404, "Not Found", load_html_file("www/404.html"));
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

void dir_or_file(std::string &fullpath, int clientFd)
{
	struct stat st;
	if(stat(fullpath.c_str() , &st) != 0)
	{
		send_response(clientFd, 404, "Not Found", load_html_file("www/404.html"));
		return ;
	}
	if(S_ISDIR(st.st_mode))
	{
		if (fullpath[fullpath.size() - 1] != '/')
		{
			send_response(clientFd, 409, "conflict", load_html_file("www/409.html"));
			return ;
		}
		if(!is_directory_empty(fullpath))
		{
			if (!delete_dir_recursive(fullpath, clientFd))
			{
				send_response(clientFd, 500, "Internal Server Error", "<html><body><h1>500 Internal Server Error</h1><p>Could not recursively delete directory.</p></body></html>");
				return;
			}
			send_response(clientFd, 204, "No Content", load_html_file("www/204.html"));
			return ;
		}
		if(remove(fullpath.c_str()) == 0) //dir empty remove it 
		{
			send_response(clientFd, 204, "No Content", load_html_file("www/204.html"));
			return ;
		}
		else  // Failed to remove directory
		{
			std::ostringstream oss;
			oss << "<html><body><h1>500 Internal Server Error</h1>"
				<< "<p>Failed to delete directory: " << strerror(errno) << "</p></body></html>";
			send_response(clientFd, 500, "Internal Server Error", oss.str());
			return ;
		}
	}
	else if (S_ISREG(st.st_mode))
	{
		if(access(fullpath.c_str(), W_OK) != 0)  // doesn't have permission for delete
		{
			send_response(clientFd, 403, "Forbidden", load_html_file("www/403.html"));
			return;
		}
		if (std::remove(fullpath.c_str()) == 0)
		{
			send_response(clientFd, 204, "No Content", load_html_file("www/204.html"));
			std::cout << "✅ File deleted: " << fullpath << std::endl;
			return;
		}
		else
		{
			std::ostringstream oss;
			oss << "<html><body><h1>500 Internal Server Error</h1>"
				<< "<p>Failed to delete directory: " << strerror(errno) << "</p></body></html>";
			send_response(clientFd, 500, "Internal Server Error", oss.str());
			return;
		}
	}
	else //Unsupported file type (e.g., socket, symlink)
	{
		send_response(clientFd, 403, "Forbidden", load_html_file("www/403.html"));
		return;
	}
}

void handle_delete_methode(request r, std::vector<ServerConfig> _configs, int clientFd, size_t conf_i)
{
	if (r.get_path().find("..") != std::string::npos) //If you do not block paths containing .., an attacker might delete or access files outside the allowed web root directory, leading to security vulnerabilities.
	{
		send_response(clientFd, 400, "Bad Request", "<html><body><h1>400 Bad Request</h1><p>Invalid path.</p></body></html>");
		return;
	}
	std::map<int, std::string> map;
    map = getMatchingRootPath(r, _configs[conf_i]);
    int key = map.begin()->first;
    if (!CheckMethodeIsAllowed("DELETE", _configs, conf_i, key))
    {
        send_response(clientFd, 405, "Method Not Allowed", load_html_file("www/405.html"));
        return;
    }
    std::string fullpath = map.begin()->second;
    std::cout << "\nFull path is:" << fullpath << std::endl;
    dir_or_file(fullpath, clientFd);
}


void send_repons_post(int clientFd, const reponse& repo)
{
	std::ostringstream response;
	response << "HTTP/1.1 " << repo.reponse_status << " OK\r\n";
	response << "Content-Length: " << repo.response_body.size() << "\r\n";
	response << "Content-Type: text/plain\r\n";
	response << "\r\n";
	response << repo.response_body;
	send(clientFd, response.str().c_str(), response.str().size(), 0);
}

void handle_post_methode(request & r, std::vector<ServerConfig> _configs, int clientFd, size_t conf_i)
{
	std::map<int, std::string> map;
	reponse repo;

    map = getMatchingRootPath(r, _configs[conf_i]);
    if (!CheckMethodeIsAllowed("POST", _configs, conf_i, map.begin()->first))
    {
        send_response(clientFd, 405, "Method Not Allowed", load_html_file("www/405.html"));
        return;
    }
    std::string fullpath = map.begin()->second;
    std::cout << "\nFull path is:" << fullpath << std::endl;
    if(r.get_path() == "/upload")
    {
        if ((long)r.body.size() > _configs[conf_i].client_max_body_size)
        {
            send_response(clientFd, 413, "Payload Too Large", load_html_file("www/413.html"));
            return ;
        }
        std::ostringstream filename;
        filename << fullpath << "/upload_" << std::time(0) << "_" << rand() <<".bin";
        std::ofstream out(filename.str().c_str(),std::ios::binary);
        if(!out)
        {
            std::cerr << "❌ Failed to open file: " << filename.str() << std::endl;
            send_response(clientFd, 500, "Internal Server Error", load_html_file("www/500.html"));
            return;
        }
        out .write(r.body.data(), r.body.size());
        out.close();
        send_response(clientFd, 201, "Created", load_html_file("www/201.html"));
    }
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