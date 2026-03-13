#pragma once
#include "Header.hpp"
#include "Request.hpp"

class Client {
	public:
		enum State {
			READING,
			WRITING,
			CLOSED
		};
	private:
		int _fd; // refers to the client's socket
		State _state;// the current state of the connexion
		std::string _readBuffer;// contain the client's http request
		std::string _writeBuffer;// contain the server's response
		Request _request;

	public:
		Client(int fd); // open the connexion
		~Client();// close the connexion

		// Public methods
		bool	readFromSocket();// read the client's request
		bool	writeToSocket();// send the response to the client
		// std::string readFile(const std::string &file);
		// void Client::handleGet(Response &res);

		// Getters Setters
		void	setState(State s);
		State	getState() const;
		int		getFd() const;
};
std::string getContentType(const std::string &path);
//Goal: To handle the client's connection
