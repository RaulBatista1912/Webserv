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
	int bytes = recv(_fd, buffer, 4096, 0); // receive the client's request maybe in piece(oui)

	if (bytes <= 0)
		return false;
	_readBuffer.append(buffer, bytes);
	size_t header_end = _readBuffer.find("\r\n\r\n");

	if (header_end != std::string::npos) {
		if (!_request.parse(_readBuffer.substr(0, header_end + 4)))
			return false;
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
				std::cout << "Path: " << _request.getPath() << std::endl;
				std::string path = _request.getPath();
				if (path == "/")
					path = "/index.html";
				std::string file = "www" + path;
				std::cout << "Server is searching: " << file << std::endl;
				std::ifstream webPage(file.c_str());
				std::string body;
				if (webPage)
				{
					std::string line;
					while (std::getline(webPage, line))
						body += line + "\n";
				}
				else
					body = "404 Not Found Honey";
				_writeBuffer = res.buildResponse(body);
				_state = WRITING;
			}
		}
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

void    Client::setState(State s) {
	_state = s;
}

Client::State Client::getState() const {
	return (_state);
}

int Client::getFd() const {
	return (_fd);
}
