#pragma once
#include "Header.hpp"
#include "Request.hpp"
#include "Config.hpp"
#include "Session.hpp"
#include "Server.hpp"

struct HttpResult {
	std::string body;
	std::string status;
	std::string contentType;
	std::map<std::string, std::string> headers;
	size_t		contentLength;
};

class Client {
	public:
		enum State {
			READING,
			WRITING,
			CLOSED
		};
	private:
		int					_fd;					// refers to the client's socket
		//int					_port;
		State				_state;					// the current state of the connexion
		std::string			_readBuffer;			// contain the client's http request
		std::string			_writeBuffer;			// contain the server's response
		std::string			_queryString;
		Request				_request;
		Config&				_config;
		Server*				_server;

	public:
		Client(int fd, Config& config, Server* server);	// open the connexion
		~Client();									// close the connexion

		// Public methods
		bool				readFromSocket();	// read the client's request
		bool				writeToSocket();	// send the response to the client
		std::string			handleRequest(size_t body_len);
		HttpResult 			handleCGI(const std::string& path, const ServerConfig* server, const Location* loc); // magie raciste
		HttpResult			handleGET(std::string& path, const ServerConfig* server, const Location* loc);
		HttpResult			handleHEAD(std::string& path, const ServerConfig* server, const Location* loc);
		HttpResult			handleDELETE(const std::string& path, const ServerConfig* server, const Location* loc);
		HttpResult			handlePOST(const std::string& path, const ServerConfig* server, const Location* loc);
		HttpResult			handleUpload(const std::string& path, const ServerConfig* server, const Location* loc);
		void 				debugRequest(const std::string &file);
		const ServerConfig*	findServer() const;
		HttpResult			handleAutoindex(const ServerConfig* server, std::string& path);

		// Getters Setters
		void				setState(State s);
		State				getState() const;
		int					getFd() const;
};
//Goal: To handle the client's connection
