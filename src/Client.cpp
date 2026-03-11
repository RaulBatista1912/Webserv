#include "../includes/Client.hpp"
#include "../includes/Response.hpp"

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
	
	// debuging
	if (_readBuffer.find("\r\n\r\n") != std::string::npos) {
		if (_request.parse(_readBuffer)) { 
			std::cout << "Method: " << _request.getMethod() << std::endl;
			std::cout << "Path: " << _request.getPath() << std::endl;
			std::cout << "Version: " << _request.getVersion() << std::endl;
			std::cout << "Body: " << _request.getBody() << std::endl;
			std::map<std::string, std::string> headers = _request.getHeaders();
			for (std::map<std::string, std::string>::iterator it = headers.begin(); it != headers.end(); ++it) {
				std::cout << it->first << ": " << it->second << std::endl;
			}
			std::string file = "../www" + _request.getPath();
			std::ifstream webPage(file.c_str());
			std::string body;
			if (webPage)
			{
				std::cout << "Entrez" << std::endl;
				std::string body;
				std::string line;

				while (std::getline(webPage, line))
				{
					body += line + "\n";
				}
				std::cout << body << std::endl;
			}
			else
			{
				body = "404 Not Found";
			}
			_writeBuffer = res.buildResponse(body);
			_state = WRITING;
		}
	}
	_writeBuffer = res.buildResponse("Hello Webserv");
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

void    Client::setState(State s) {
	_state = s;
}

Client::State Client::getState() const {
	return (_state);
}

int Client::getFd() const {
	return (_fd);
}
