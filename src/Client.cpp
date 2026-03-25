#include "../includes/Client.hpp"
#include "../includes/Response.hpp"
#include <iostream>
#include <cstdlib>

Client::Client(int fd, const std::string& root, const std::string& index): 
_fd(fd), _state(READING), _root(root), _index(index){}

Client::~Client() {
	if (_fd >= 0)
		close(_fd);
}

// Public methods
std::string readFile(const std::string& path) {
	std::ifstream file(path.c_str());
	std::stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}

HttpResult Client::handleCGI()
{
    HttpResult r;

    std::string fullPath = _root + _request.getPath();

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
        r.contentType = "text/plain"; // temp

        return r;
    }
}

HttpResult Client::handlePOST() {
	HttpResult r;

	// 1. Récupérer le body
	std::string body = _request.getBody();

	// 2. Construire le chemin du fichier
	std::string path = _root + _request.getPath();

	// 3. Sécurité basique
	if (path.find("..") != std::string::npos) {
		r.status = "403 Forbidden";
		r.body = "<h1>403 Forbidden</h1>";
		r.contentType = "text/html";
		return r;
	}

	// 4. Ouvrir le fichier en écriture
	std::ofstream out(path.c_str(), std::ios::binary);
	if (!out) {
		r.status = "500 Internal Server Error";
		r.body = "<h1>500 Internal Server Error</h1>";
		r.contentType = "text/html";
		return r;
	}

	// 5. Écrire le body
	out.write(body.c_str(), body.size());
	out.close();

	// 6. Réponse
	r.status = "201 Created";
	r.body = "<h1>File uploaded</h1>";
	r.contentType = "text/html";
	return r;
}


HttpResult Client::handleGET() {
	HttpResult r;
	std::string file;
	std::string path = _request.getPath();

	if (path.find("..") != std::string::npos) {
		r.body = "<h1>403 Forbidden</h1>";
		r.status = "403 Forbidden";
		r.contentType = "text/html";
		return r;
	}
	if (path == "/")
		path = "/" + _index;
	file = _root + path;
	//debug
	debugRequest(file);
	if (path.find(".cgi") != std::string::npos)
		return handleCGI();
	std::ifstream webPage(file.c_str(), std::ios::binary);
	if (webPage) {
		std::stringstream buffer;
		buffer << webPage.rdbuf();
		r.body = buffer.str();
		r.status = "200 OK";
		r.contentType = getContentType(path);
	} else {
		r.body = readFile("www/error_page/404.html");
		if (r.body.empty())
			r.body = "<h1>404 Not Found</h1>";
		r.status = "404 Not Found";
		r.contentType = "text/html";
	}
	return r;
}


std::string Client::handleRequest() {
	Response res;
	HttpResult r;
	std::string method = _request.getMethod();

	if (method == "GET")
		r = handleGET();
	else if (method == "POST")
		r = handlePOST();
	else if (method == "DELETE") {
		r.body = "<h1>DELETE not implemented yet</h1>";
		r.status = "405 Method Not Allowed";
		r.contentType = "text/html";
	}
	else {
		r.body = "<h1>405 Method Not Allowed</h1>";
		r.status = "405 Method Not Allowed";
		r.contentType = "text/html";
	}
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
	return "text/plain";
}