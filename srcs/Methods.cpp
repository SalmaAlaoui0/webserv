#include "../includes/Server.hpp"


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


std::map<int, std::string> getMatchingRootPath(request &r, const ServerConfig &config)
{
	std::string requestedPath = r.get_path(); // e.g. "/index.html"
	std::string matchedRoot;
	size_t maxMatchLength = 0;
	std::string locPath;
	std::vector<std::string> Pchunks;
	size_t i = 0;

	std::map<int, std::string> result;
	for (i = 0; i < config.locations.size(); ++i)
	{
		locPath = config.locations[i].path;
		Pchunks = pathchunks(locPath);
		if (Pchunks.size() == 1)
			locPath = Pchunks[0];
		else
			locPath = Pchunks[1];
		if (Pchunks.size() == 2 && Pchunks[0] == "^~")
		{
			if (requestedPath.find(locPath) == 0)
			{
				matchedRoot = config.locations[i].root + requestedPath.substr(locPath.length());
				result[i] = matchedRoot;
				return result;
			}
		}		
		else if (requestedPath.find(locPath) == 0 && locPath.length() > maxMatchLength)
		{
			maxMatchLength = locPath.length();
			matchedRoot = config.locations[i].root + requestedPath.substr(locPath.length());
		}
	}
	result[i] = matchedRoot;
	return result;
}

void handle_get_methode(request r, const std::vector<ServerConfig> _configs)
{
	int port = 8080;
	std::map<int, std::string> map;

	for (size_t i = 0; i < _configs.size(); ++i)
	{
		if (_configs[i].port == port)
		{
			map = getMatchingRootPath(r, _configs[i]);
			for (std::map<int, std::string>::const_iterator it = map.begin(); it != map.end(); ++it) {
			std::cout << it->first << ": " << it->second;
			std::map<int, std::string>::const_iterator next = it;
			++next;
			if (next != map.end()) {
				std::cout << ", ";
			}
		}

			// 👉 Next step: check if file exists, open it, and send response

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
