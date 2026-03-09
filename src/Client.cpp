#include "../includes/Client.hpp"
#include <iostream>

Client::Client(int fd) : fd(fd), state(READING) {

}

Client::~Client() {
	if (fd >= 0)
		close(fd);
}

bool Client::readFromSocket() {
	char buffer[4096];
	int bytes = recv(fd, buffer, 4096, 0);

	if (bytes <= 0)
		return false;
	readBuffer.append(buffer, bytes);

	if (readBuffer.find("\r\n\r\n") != std::string::npos) {
		if (_request.parse(readBuffer)) {
			std::cout << "Method: " << _request.getMethod() << std::endl;
			std::cout << "Path: " << _request.getPath() << std::endl;
			std::cout << "Version: " << _request.getVersion() << std::endl;
		}
	}
	return true;
}

bool Client::writeToSocket() {
	if (writeBuffer.empty()) {
		state = CLOSED;
		return (false);
	}
	ssize_t sent = send(fd, writeBuffer.c_str(), writeBuffer.size(), 0);
	if (sent <= 0) {
		state = CLOSED;
		return (false);
	}
	writeBuffer.erase(0, sent);
	if (writeBuffer.empty())
		state = CLOSED;
	return (true);
}

void    Client::setState(State s) {
	state = s;
}

Client::State Client::getState() const {
	return (state);
}

int Client::getFd() const {
	return (fd);
}
