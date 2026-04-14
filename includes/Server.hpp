#pragma once
#include "Header.hpp"

class Server {
	private:
		int _fd;// server's socket
		int _port; // port where the server listens for incoming connections
		std::string _root;
	public:
		Server(int port, const std::string& root);
		~Server();

		// Getters Setters
		int getFd() const;
		int getPort() const;
		const std::string& getRoot() const;
		const std::string& getIndex() const;

		// Public methods
		int acceptClient() const;
};
//Goal: To listen the incoming connections
