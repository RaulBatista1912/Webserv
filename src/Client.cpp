#include "../includes/Client.hpp"
#include "../includes/Response.hpp"

Client::Client(int fd) : fd(fd), state(READING) {

}

Client::~Client() {
	if (fd >= 0)
		close(fd);
}

bool Client::readFromSocket() {
	char buffer[4096];
	Response res;
	ssize_t bytes = recv(fd, buffer, 4096, 0);
	if (bytes > 0) {
		readBuffer.append(buffer, bytes);
		if (readBuffer.find("\r\n\r\n") != std::string::npos) {
			writeBuffer = res.buildResponse("Hello");
			state = WRITING;
		}
		return (true);
	}
	state = CLOSED;
	return (false);
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
