#include "../includes/Client.hpp"
#include "../includes/Response.hpp"
#include <iostream>
#include <cstdlib>

Client::Client(int fd) : _fd(fd), _state(READING) {

}

Client::~Client() {
	if (_fd >= 0)
		close(_fd);
}

bool Client::readFromSocket() {
	char buffer[4096];
	Response res;
	std::string body;
	int status;
	int bytes = recv(_fd, buffer, 4096, 0); // receive the client's request maybe in piece(oui)

	if (bytes <= 0)
		return false;
	_readBuffer.append(buffer, bytes);
	size_t header_end = _readBuffer.find("\r\n\r\n");

	if (header_end != std::string::npos) {
		if (!_request.parse(_readBuffer.substr(0, header_end + 4))) {
			body = "<h1>400 Bad Request</h1>";
			status = 400;
			_writeBuffer = res.buildResponse(body);
			_state = WRITING;
			return true;
		}
		std::map<std::string, std::string> headers = _request.getHeaders();
		size_t body_len = 0;
		if (headers.count("Content-Length"))
			body_len = std::atoi(headers["Content-Length"].c_str());
		if (_readBuffer.size() >= header_end + 4 + body_len) {
			if (_request.parse(_readBuffer)) {
				std::cout << "Method: " << _request.getMethod() << std::endl;
				std::cout << "Path: " << _request.getPath() << std::endl;
				std::cout << "Version: " << _request.getVersion() << std::endl;

				std::map<std::string, std::string> headers = _request.getHeaders();
				for (std::map<std::string, std::string>::iterator it = headers.begin(); it != headers.end(); ++it)
					std::cout << it->first << ": " << it->second << std::endl;

				std::cout << "Body: " << _request.getBody() << std::endl;
				//Trying to read the index.html file
				std::string path = _request.getPath();
				if (path == "/")
					path = "/index.html";
				std::string file = "www" + path; // i have to handle in default_config file not here
				std::cout << "Server is searching: " << file << std::endl;
				std::ifstream webPage(file.c_str());
				if (webPage) {
					std::string line;
					while (std::getline(webPage, line))
						body += line + "\n";
					status = 200;
				}
				else {
					body = "<h1>404 Not Found</h1>";
					status = 404;
				}
			}
		}
		_writeBuffer = res.buildResponse(body);
		_state = WRITING;
	}
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

// std::string	Client::readFile(const std::string &file) {
// 	std::ifstream webPage(file.c_str());
// 	std::string body;
// 	std::string line;

// 	while (std::getline(webPage, line))
// 		body += line + "\n";

// 	return body;
// }

// void	Client::handleGet(Response &res) {
// 	std::string path = _request.getPath();

// 	if (path == "/")
// 		path = "/index.html";

// 	std::string file = "www" + path;

// 	std::ifstream webPage(file.c_str());

// 	std::string body;
// 	int status;

// 	if (webPage) {
// 		body = readFile(file);
// 		status = 200;
// 	}
// 	else {
// 		body = "<h1>404 Not Found</h1>";
// 		status = 404;
// 	}

// 	_writeBuffer = res.buildResponse(body);
// 	_state = WRITING;
// }

void	Client::setState(State s) {
	_state = s;
}

Client::State Client::getState() const {
	return (_state);
}

int Client::getFd() const {
	return (_fd);
}
