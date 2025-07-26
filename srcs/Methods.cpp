#include "../includes/Server.hpp"
#include "../includes/Response.hpp"
#include "../includes/ServerConfig.hpp"
#include "../includes/LocationConfig.hpp"


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
	std::cout << "************" << requestedPath << std::endl;

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
			std::cout << "And the locpath is: " << locPath << "----- and it's len is: " << locPath.length() << std::endl;
		}
		else
			locPath = Pchunks[1];
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
			std::cout << "CODE GOT TILL HERE hh but dono what to do\n" << std::endl;
			maxMatchLength = locPath.length();
			matchedRoot = join_path(config.locations[i].root, requestedPath.substr(locPath.length()));
			coorLoc = i;
			// std::cout << "root is: -" << config.locations[i].root << "- and req root is: -" << requestedPath.substr(locPath.length()) << "--\n";
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
	std::cout << "HELLO WORLD THE LOC NUM IS: " << locationum << ", and it's in server: " << servernum << std::endl;
	(void)method;
	it = _configs[servernum].locations[locationum].allowed_methods.begin();
	while (it != _configs[servernum].locations[locationum].allowed_methods.end())
	{
		if (method == *it)
			return 1;
		// std::cout << "Methodes are: ^^^" << *it << std::endl;
		++it;
	}
	return 0;
}


// std::string send_error(int errorNum)
// {
// 	std::string result;
// 	result = ;
// 	return result;
// }

std::string CheckDirOrFile(std::string requested_path, int clientFd, std::vector<LocationConfig> config, int key)
{
	struct stat statbuf;
	// std::cout << "\n\n Your fileis :" << requested_path << "\n\n";
    if (stat(requested_path.c_str(), &statbuf) == 0) {
        if (S_ISREG(statbuf.st_mode)) {
            // ✅ It's a file → serve it
			return send_file_response(clientFd, requested_path);
        } else if (S_ISDIR(statbuf.st_mode)) {
            // 📁 It's a directory → try index
            // std::string index_file = requested_path + "/" + config[key].index;
			
			std::string index_file;
			index_file = join_path(requested_path, config[key].index);
            if (stat(index_file.c_str(), &statbuf) == 0 && S_ISREG(statbuf.st_mode)) {
				// std::cout << "the index is: " << index_file << "\n\n";
                return send_file_response(clientFd, index_file);
            } else if (config[key].autoindex) {
                // return serve_autoindex_listing(requested_path);
				return "still didn't handle autoindex yet\n";
            } else {
				std::cout << "1Hello beautiful world the error is: " << 403 << std::endl;
                // return send_error(403); // Forbidden (index off and no index file)
            }
        } else {
			std::cout << "2Hello beautiful world the error is: " << 403 << std::endl;
            // return send_error(403); // Not a regular file or dir
        }
    } else {
		std::cout << "3Hello beautiful world the error is: " << 404 << std::endl;
        // return send_error(404); // ❌ Not found
    }
	return "  *";
}


void handle_get_methode(request r, std::vector<ServerConfig> _configs, int clientFd)
{
	int port = 8080;
	std::map<int, std::string> map;

	for (size_t i = 0; i < _configs.size(); ++i)
	{
		if (_configs[i].port == port)
		{
			map = getMatchingRootPath(r, _configs[i]);///////Hereeeeeeeee
			int key = map.begin()->first;
			// std::cout << map.size() << std::endl;
			if (!CheckMethodeIsAllowed("GET", _configs, i, key))
				std::cout << "This means method not allowed \n\n" << std::endl;
			// here we should return the METHODE NOT ALLOWED ERROR
			else
				std::cout << "Yaaay method found this means method allowed\n\n" << std::endl;
			std::string value = map.begin()->second;
			std::cout << "\nFull path is:" << value << std::endl;
			std::string ret = CheckDirOrFile(value, clientFd, _configs[i].locations, key);
			std::cout << ret << std::endl;

			// std::cout << "***&&&" << value << "&&&***" << std::endl;



// done ✅ : // 1- Lbnat here the file is being readed and the correct location with the correct path are being send back in a map container type <int, string>
// done ✅ : // 2- Now I will check if the methode asked from the request is avialable in the location and
			 // 3- then I will check if the path is valid with the stat (system function)
			 // 4- Lastly if those instructions are passed this mean we're good to start with the methods

			//  ⬇️ ⬇️ ⬇️ ⬇️ ⬇️ ⬇️ ⬇️ 
			/// ****PRINT THE MAP VALUES****

			// for (std::map<int, std::string>::iterator it = map.begin(); it != map.end(); ++it) {
			// std::cout << it->first << ": -" << it->second << "-\n";
			// std::map<int, std::string>::iterator next = it;
			// ++next;
			// if (next != map.end()) {
			// 	std::cout << ", ";
			// }
		// }

			// 👉 Next step: check if file exists, open it, and send response

			return;
		}
	}
}


void handle_post_methode(request & r, const std::vector<ServerConfig> _configs, int clientFd)
{
	const size_t client_max_body_size = 10 * 1024 * 1024;
	int port = 8080;
reponse repo;
	for (size_t i = 0; i < _configs.size(); ++i)
	{
		if (_configs[i].port == port)
		{
			/// Salam Mouna I chanded the return value dial had lfunction ⬇️ to return map blast string (chofi lfunction diali kifach kanprinti l values dial lmap)
			std::string fullPath = getMatchingRootPath(r, _configs[i]);// getMatchingRootPath
			std::cout << "Full path to serve: " << fullPath << std::endl;
			std::ostringstream filename ;
			filename << fullPath << "/upload_" << std::time(0) << ".txt";
			std::ofstream out(filename.str().c_str(),std::ios::binary);
			if(!out)
			{
				std::cerr << "❌ Failed to open file: " << filename.str() << std::endl;
				repo.reponse_status = 500;
				repo.response_body = "Internal Server Error";
				return;
		
			}
			out << r.body;
			out.close();
			if(r.body.size() > client_max_body_size)
			{
				repo.reponse_status = 413;
				repo.response_body =" Payload Too Large.";
				return ;
			}
			/////////////chek method allowod or not
			std::cout << "✅ Body written to: " << filename.str() << std::endl;
			repo.reponse_status = 201;
			repo.response_body = "File uploaded successfully!\n";


    ssize_t sent = send(clientFd, repo.response_body.c_str(), repo.response_body.size(), 0);
    if (sent < 0)
        std::cerr << "❌ send failed: " << strerror(errno) << std::endl;
    else
        std::cout << "Response sent to FD: " << clientFd << std::endl;

			return;
		}
	}
}











// void SplitPath(std::string requestPath, std::vector<std::string> &parts)
// {
// 	std::stringstream ss(requestPath);
// 	std::string split;
	
// 	while (std::getline(ss, split, '/'))
// 	{
// 		if (!split.empty())
// 			parts.push_back(split);
// 	}
// }

// int CorrectPath(std::vector<std::string> paths, std::string requestroot)
// {
// 	size_t i = 0;
// 	if (!requestroot.empty())
// 	{
// 		while (i < paths.size())
// 		{
// 			if (paths[i] == requestroot)
// 			{
// 				std::cout << "found index: " << i << std::endl;
// 				return i; // just returning the index of the location
// 				/* what is waiting for u next */

// 				/* Use stat() to find out what kind of resource the path is:
// 				Is it a regular file?

// 				Is it a directory?

// 				Based on that:

// 				👉 If it’s a regular file:
// 				→ Open, read it, and send the contents back with a 200 OK response.

// 				👉 If it’s a directory:

// 				If there's an index file configured (like index.html) → look for it and serve it.

// 				If no index is found → check if autoindex is enabled and generate a directory listing.

// 				Otherwise → send 403 Forbidden.

// 				👉 If stat() fails (file not found) → return 404 Not Found. */
// 			}
// 			i++;
// 		}
// 	}
// 	return 1;
// }

// void handle_get_methode(request r, std::vector<ServerConfig> _configs)
// {
// 	size_t i = 0;
// 	size_t j = 0;
// 	std::vector<std::string> paths;
//     std::cout << "Started appliquing Method -: " << r.get_method() << std::endl;
// 	//supposed all of this if coming from server with port num 8080
// 	int port = 8080;
// 	while (i < _configs.size())
// 	{
// 		if (_configs[i].port == port)
// 		{
// 			j = 0;
// 			while (j < _configs[i].locations.size())
// 			{
// 				paths.push_back(_configs[i].locations[j].path);
// 				j++;
// 			}
// 			int correspondantPath = CorrectPath(paths, r.get_path());
// 			std::cout << "the data I should respond with is: " << _configs[i].locations[correspondantPath].index << std::endl << std::endl;
// 			// send the appropriate respond from the appropriate server's location
// 		}
// 		i++;
// 	}
// }
