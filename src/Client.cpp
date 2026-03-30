#include "../includes/Client.hpp"
#include "../includes/Response.hpp"
#include "../includes/ServerConfig.hpp"
#include <iostream>
#include <cstdlib>

Client::Client(int fd, Config& config): _fd(fd), _state(READING), _config(config){}

Client::~Client() {
	if (_fd >= 0)
		close(_fd);
}

// Public methods
HttpResult Client::handleCGI(std::string& path, const ServerConfig* server, const Location* loc)
{
	HttpResult r;
	(void)loc;
	std::string fullPath = server->root + _request.getPath();

	int pipefd[2];
	if (pipe(pipefd) == -1) {
		r.status = "500 Internal Server Error";
		r.body = "pipe error";
		return r;
	}
	pid_t pid = fork();
	if (pid == 0) {		//ENFANT
		dup2(pipefd[1], STDOUT_FILENO);
		close(pipefd[0]);
		close(pipefd[1]);

		// recupere la variable apres le ? pour la fournir au cgi;
		std::string query = "";
		size_t pos = _request.getPath().find("?");
		if (pos != std::string::npos)
			query = _request.getPath().substr(pos + 1);

		setenv("QUERY_STRING", query.c_str(), 1);

		char *argv[] = {(char *)fullPath.c_str(), NULL};
		execve(fullPath.c_str(), argv, NULL);
		exit(1);
	}
	else
	{
		// PARENT
		close(pipefd[1]);
		char buffer[4096];
		std::string output;
		int bytes = 0;

		while ((bytes = read(pipefd[0], buffer, sizeof(buffer))) > 0) // envoie la rep
			output.append(buffer, bytes);
		close(pipefd[0]);
		waitpid(pid, NULL, 0);

		r.status = "200 OK";
		r.body = output;
		r.contentType = getContentType(path);
		return r;
	}
}

HttpResult Client::handlePOST(std::string& path, const ServerConfig* server, const Location* loc) {
	HttpResult r;
	std::string contentType = _request.getHeader("Content-Type");
	(void)loc;
	// 1) Vérifier si c'est un upload
	// a checker la longueur de content length > maxBodySize
	if (contentType.find("multipart/form-data") != std::string::npos) {
		std::string body = _request.getBody();

		// 2) Récupérer le boundary
		size_t pos = contentType.find("boundary=");
		std::string boundary = "--" + contentType.substr(pos + 9);

		// 3) Trouver la première partie
		size_t start = body.find(boundary);
		start += boundary.size() + 2;

		// 4) Headers de la partie
		size_t headerEnd = body.find("\r\n\r\n", start);
		std::string headers = body.substr(start, headerEnd - start);

		// 5) Extraire filename
		std::string filename;
		size_t fn = headers.find("filename=\"");
		fn += 10;
		size_t end = headers.find("\"", fn);
		filename = headers.substr(fn, end - fn);

		// 6) Extraire contenu du fichier
		size_t fileStart = headerEnd + 4;
		size_t fileEnd = body.find(boundary, fileStart) - 2;
		std::string fileContent = body.substr(fileStart, fileEnd - fileStart);

		// 7) Écrire le fichier
		std::string filepath = server->root + path + "/" + filename;
		int fd = open(filepath.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0644);
		if (fd < 0)
			return handleError(server, 404, "404 Not Found");
		if (isDirectory(filepath))
			return handleError(server, 403, "403 Forbidden");
		write(fd, fileContent.c_str(), fileContent.size());
		close(fd);
		return handleError(server, 201, "201 Created");
	}

	// 1. Récupérer le body
	std::string body = _request.getBody();
	std::string value;

	size_t pos = body.find('=');
	if (pos != std::string::npos)
		value = body.substr(pos + 1);
	// 2. Construire le chemin du fichier
	std::string completePath = server->root + path;

	//debug
	//debugRequest(completePath);
	// 3. Sécurité basique
	if (completePath.find("..") != std::string::npos || isDirectory(completePath))
		return handleError(server, 403, "403 Forbidden");
	// 4. Ouvrir le fichier en écriture
	std::ofstream out(completePath.c_str(), std::ios::binary);
	if (!out)
		return handleError(server, 500, "500 Internal Server Error");
	// 5. Écrire le body
	out.write(value.c_str(), value.size());
	out.close();
	// 6. Réponse
	return handleError(server, 201, "201 Created");
}

const ServerConfig*	Client::findServer() const {
	const std::vector<ServerConfig>& servers = _config.getServers();
	const std::map<std::string, std::string>& headers = _request.getHeaders();
	std::map<std::string, std::string>::const_iterator it = headers.find("Host");

	if (it == headers.end() || it->second.empty())
		return &servers[0];

	std::string host = it->second;
	std::size_t pos = host.find(':');
	int	port = 0;
	if (pos != std::string::npos) {
		std::string portStr = host.substr(pos + 1);
		port = std::atoi(portStr.c_str());
	}

	for (size_t i = 0; i < servers.size(); ++i) {
		const ServerConfig& s = servers[i];
		if (port == s.port)
			return &s;
	}
	std::cout << std::endl;
	return &servers[0];
}

std::string	Client::readErrorPage(const ServerConfig& server, int code) {
	std::map<int, std::string>::const_iterator it = server.errorPages.find(code);

	if (it != server.errorPages.end()) {
		std::string filePath = server.root + "/" + it->second;

		std::ifstream file(filePath.c_str(), std::ios::binary);
		if (file) {
			std::stringstream buffer;
			buffer << file.rdbuf();
			return buffer.str();
		}
	}
	return "<h1>Error</h1>";
}

HttpResult Client::handleError(const ServerConfig* server, int code, const std::string& err) {
	HttpResult r;

	if (server->allowErrPage && server->errorPages.find(code) != server->errorPages.end()) 
		r.body = readErrorPage(*server, code);
	else
		r.body = "<h1>" + err + "<h1>";
	r.status = err;
	r.contentType = "text/html";
	return r;
}

#include <dirent.h>   // opendir, readdir, closedir
#include <sys/stat.h> // stat
#include <string>
#include <sstream>

HttpResult Client::handleAutoindex(const ServerConfig* server, std::string path) {
    HttpResult r;

    std::stringstream html;

    html << "<html><head><title>Index of " << path << "</title></head><body>";
    html << "<h1>Index of " << path << "</h1><ul>";

    DIR* dir = opendir(path.c_str());
    if (dir == NULL) {
        return handleError(server, 500, "500 Internal Server Error");
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        std::string name = entry->d_name;

        // ignorer "." et ".."
        if (name == "." || name == "..")
            continue;

        html << "<li><a href=\"" << name << "\">" << name << "</a></li>";
    }

    closedir(dir);

    html << "</ul></body></html>";

    r.body = html.str();
    r.status = "200 OK";
    r.contentType = "text/html";

    return r;
}


HttpResult Client::handleGET(std::string& path, const ServerConfig* server, const Location* loc) {
	HttpResult r;
	std::string file;

	if (path.find("..") != std::string::npos) {
		r = handleError(server, 403, "403 Forbidden");
		return r;
	}
	if (loc && !loc->allowGet) {
		r = handleError(server, 405, "405 Method Not Allowed");
		return r;
	}
	if (path == "/")
		path = "/" + server->index;
	else if (isDirectory(server->root + path)) {
		if (loc->autoindex) {
			r = handleAutoindex(server, server->root + path);
			return r;
		}
		else {
			r = handleError(server, 403, "403 Forbidden");
			return r;
		}
	}
	file = server->root + path;
	//debug
	//debugRequest(file);
	if (path.find(".cgi") != std::string::npos)
		return handleCGI(path, server, loc);
	std::ifstream webPage(file.c_str(), std::ios::binary);
	if (webPage) {
		//std::cout << "OPEN FILE: " << file << "\n" << std::endl;
		std::stringstream buffer;
		buffer << webPage.rdbuf();
		r.body = buffer.str();
		r.status = "200 OK";
		r.contentType = getContentType(path);
	}
	else
		r = handleError(server, 404, "404 Not Found");
	return r;
}

std::string Client::handleRequest() {
	Response res;
	HttpResult r;
	std::string method = _request.getMethod();
	//std::cout << method << std::endl;
	std::string path = _request.getPath();
	//std::cout << path << std::endl;
	const ServerConfig* server = findServer();
	//std::cout << server->port << std::endl;
	const Location* loc = server->findLocation(path);
	//std::cout << loc << std::endl;

	if (method == "GET")
		r = handleGET(path, server, loc);
	else if (method == "POST")
		r = handlePOST(path, server, loc);
	else if (method == "DELETE")
		r = handleError(server, 501, "501 Not Implemented");
	else
		r = handleError(server, 501, "501 Not Implemented");
	return res.buildResponse(r.status, r.body, r.contentType);
}


bool Client::readFromSocket() {
	char buffer[4096];
	int bytes = recv(_fd, buffer, sizeof(buffer), 0);

	if (bytes <= 0)
		return false;
	_readBuffer.append(buffer, bytes);
	size_t header_end = _readBuffer.find("\r\n\r\n");
	if (header_end == std::string::npos)
		return true;
	size_t body_len = 0;
	std::string headers = _readBuffer.substr(0, header_end + 4);
	std::map<std::string, std::string> h = _request.extractHeaders(headers);
	if (h.count("Content-Length"))
		body_len = std::atoi(h["Content-Length"].c_str());
	if (_readBuffer.size() < header_end + 4 + body_len)
		return true;
	if (!_request.parse(_readBuffer)) {
		Response res;
		_writeBuffer = res.buildResponse("400 Bad Request",
										"<h1>400 Bad Request</h1>",
										"text/html");
		_state = WRITING;
		return true;
	}
	_writeBuffer = handleRequest();
	_state = WRITING;
	return true;
}

bool Client::writeToSocket() {
	if (_writeBuffer.empty()) {
		_state = CLOSED;
		return (false);
	}
	ssize_t sent = send(_fd, _writeBuffer.c_str(), _writeBuffer.size(), 0);
	if (sent <= 0) {
		_state = CLOSED;
		return (false);
	}
	_writeBuffer.erase(0, sent);
	if (_writeBuffer.empty())
		_state = CLOSED;
	return (true);
}

std::string readFile(const std::string& path) {
	std::ifstream file(path.c_str());
	std::stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}

bool isDirectory(const std::string &path) {
	struct stat st;
	if (stat(path.c_str(), &st) == -1)
		return false;
	return S_ISDIR(st.st_mode);
}

void Client::debugRequest(const std::string &file) {
	std::cout << "----- DEBUG REQUEST -----" << std::endl;
	std::cout << "New client fd=" << _fd << std::endl;
	std::cout << "Method:  " << _request.getMethod() << std::endl;
	std::cout << "Path:    " << _request.getPath() << std::endl;
	std::cout << "Version: " << _request.getVersion() << std::endl;
	std::cout << "\nHeaders:" << std::endl;
	std::map<std::string, std::string> headers = _request.getHeaders();
	for (std::map<std::string, std::string>::iterator it = headers.begin();
		it != headers.end(); ++it)
	{
		std::cout << "  " << it->first << ": " << it->second << std::endl;
	}
	std::cout << "\nBody:" << std::endl;
	std::cout << _request.getBody() << std::endl;
	std::cout << "\nServer is searching: " << file << std::endl;
	std::cout << "----------END REQUEST---------------\n" << std::endl;
}

// Getters Setters
void	Client::setState(State s) {
	_state = s;
}

Client::State Client::getState() const {
	return (_state);
}

int Client::getFd() const {
	return (_fd);
}

std::string getContentType(const std::string &path)
{
	if (path.find(".html") != std::string::npos)
		return "text/html";
	if (path.find(".css") != std::string::npos)
		return "text/css";
	if (path.find(".js") != std::string::npos)
		return "application/javascript";
	if (path.find(".png") != std::string::npos)
		return "image/png";
	if (path.find(".jpg") != std::string::npos)
		return "image/jpeg";
	if (path.find(".gif") != std::string::npos)
		return "image/gif";
	return "text/plain";
}
