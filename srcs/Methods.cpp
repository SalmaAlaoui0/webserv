#include "../includes/Server.hpp"
#include "../includes/Response.hpp"


std::string getMatchingRootPath(request &r, const ServerConfig &config)
{
	std::string requestedPath = r.get_path(); // e.g. "/index.html"
	std::string matchedRoot;
	size_t maxMatchLength = 0;
	std::string locPath;

	for (size_t i = 0; i < config.locations.size(); ++i)
	{
		locPath = config.locations[i].path;
		locPath = locPath.substr(0, locPath.size() - 1);

		// std::cout << "req is: -" << requestedPath << "--" << std::endl;
		// std::cout << "loc is: -" << locPath << "--" << std::endl;
		if (requestedPath == locPath)
			std::cout << "Hello world\n";
		// Check if location path is a prefix of the request path
		if (requestedPath.find(locPath) == 0 && locPath.length() > maxMatchLength)
		{
			maxMatchLength = locPath.length();
			matchedRoot = config.locations[i].root + requestedPath.substr(locPath.length());
		}
	}
	std::cout << "The request path is: " << requestedPath << "\nAnd the corr.loc path is: " << locPath << std::endl;
	std::cout << "--- and plus the rootifound is: " << matchedRoot << std::endl;
	return matchedRoot;
}

void handle_get_methode(request r, const std::vector<ServerConfig> _configs)
{
	int port = 8080;

	for (size_t i = 0; i < _configs.size(); ++i)
	{
		if (_configs[i].port == port)
		{
			std::string fullPath = getMatchingRootPath(r, _configs[i]);
			std::cout << "Full path to serve: " << fullPath << std::endl;

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
			std::string fullPath = getMatchingRootPath(r, _configs[i]);
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
