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

bool isDirectory(const std::string &path) {
	struct stat st;
	if (stat(path.c_str(), &st) == -1)
		return false;
	return S_ISDIR(st.st_mode);
}

HttpResult Client::handlePOST() {
	HttpResult r;

	std::string contentType = _request.getHeader("Content-Type");
	std::cout << "Content-Type = " << contentType << std::endl;

	// 1) Vérifier si c'est un upload
	if (contentType.find("multipart/form-data") != std::string::npos) {
		std::string body = _request.getBody();

		// 2) Récupérer le boundary
		size_t pos = contentType.find("boundary=");
		std::string boundary = "--" + contentType.substr(pos + 9);
		std::cout << "Boundary = [" << boundary << "]" << std::endl;

		// 3) Trouver la première partie
		size_t start = body.find(boundary);
		start += boundary.size() + 2;

		// 4) Headers de la partie
		size_t headerEnd = body.find("\r\n\r\n", start);
		std::string headers = body.substr(start, headerEnd - start);
		std::cout << "Body length = " << body.size() << std::endl;

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
std::cout << "File content size = " << fileContent.size() << std::endl;

		// 7) Écrire le fichier
		std::string filepath = _root + "/blabla/" + filename;
		int fd = open(filepath.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0644);
		if (fd < 0)
		{
			r.status = "404 Not Found";
			r.body = "<h1>404 Not Found</h1>";
			r.contentType = "text/html";
			return r;
		}
		if (isDirectory(filepath))
		{
			r.status = "403 Forbidden";
			r.body = "<h1>403 Forbidden</h1>";
			r.contentType = "text/html";
			return r;
		}
		write(fd, fileContent.c_str(), fileContent.size());
		close(fd);

		r.status = "201 Created";
		r.body = "<h1>File uploaded</h1>";
		r.contentType = "text/html";
		return r;
	}

	// 1. Récupérer le body
	std::string body = _request.getBody();
	std::string value;

	size_t pos = body.find('=');
	if (pos != std::string::npos)
		value = body.substr(pos + 1);
	// 2. Construire le chemin du fichier
	std::string path = _root + _request.getPath();

	//debug
	debugRequest(path);
	// 3. Sécurité basique
	if (path.find("..") != std::string::npos || !isDirectory(path)) {
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
	out.write(value.c_str(), value.size());
	out.close();

	// 6. Réponse
	r.status = "201 Created";
	r.body = "<h1>Message sent</h1>";
	r.contentType = "text/html";
	return r;
}

HttpResult Client::handleGET() {
	HttpResult r;
	std::string completePath;
	std::string path = _request.getPath();

	if (path.find("..") != std::string::npos) {
		r.body = "<h1>403 Forbidden</h1>";
		r.status = "403 Forbidden";
		r.contentType = "text/html";
		return r;
	}
	if (path == "/")
		path = "/" + _index;
	completePath = _root + path;
	//debug
	debugRequest(completePath);
	std::ifstream webPage(completePath.c_str(), std::ios::binary);
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
