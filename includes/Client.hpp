#pragma once
#include "Header.hpp"
#include "Request.hpp"
#include "Config.hpp"

struct HttpResult {
	std::string body;
	std::string status;
	std::string contentType;
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
		int					_port;
		State				_state;					// the current state of the connexion
		std::string			_readBuffer;			// contain the client's http request
		std::string			_writeBuffer;			// contain the server's response
		Request				_request;
		Config&				_config;

	public:
		Client(int fd, int port, Config& config);	// open the connexion
		~Client();									// close the connexion

		// Public methods
		bool				readFromSocket();		// read the client's request
		bool				writeToSocket();		// send the response to the client
		std::string			handleRequest();
		HttpResult			handleGET(std::string& path, const ServerConfig* server, const Location* loc);
		HttpResult			handlePOST(std::string& path, const ServerConfig* server, const Location* loc);
		void 				debugRequest(const std::string &file);
		const ServerConfig*	findServer() const;
		std::string			readErrorPage(const ServerConfig& server, int code);
		HttpResult			handleError(const ServerConfig* server, int code, const std::string& err, const std::string& path);

		// Getters Setters
		void				setState(State s);
		State				getState() const;
		int					getFd() const;
};

std::string					getContentType(const std::string &path);
bool						isDirectory(const std::string &path);
std::string					readFile(const std::string& path);
//Goal: To handle the client's connection
