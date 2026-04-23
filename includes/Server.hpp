#pragma once
#include "Header.hpp"
#include "Session.hpp"

class Server {
	private:
		int _fd;// server's socket
		int _port; // port where the server listens for incoming connections
		std::string _root;
		SessionManager _sessionManager;
	public:
		Server(int port, const std::string& root);
		~Server();

		// Public methods
		int acceptClient() const			{return (accept(_fd, NULL, NULL));}

		// Getters Setters
		int getFd() const					{return _fd;};
		int getPort() const					{return _port;}
		const std::string& getRoot() const	{return _root;}
		SessionManager& getSessionManager()	{return _sessionManager;}
};
//Goal: To listen the incoming connections
