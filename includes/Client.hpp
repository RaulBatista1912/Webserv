#pragma once
#include "Header.hpp"
#include "Request.hpp"
#include "Config.hpp"

struct HttpResult {
	std::string body;
	std::string status;
	std::string contentType;
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
		Request				_request;
		Config&				_config;

	public:
		Client(int fd, Config& config);	// open the connexion
		~Client();									// close the connexion

		// Public methods
		bool				readFromSocket();	// read the client's request
		bool				writeToSocket();	// send the response to the client
		std::string			handleRequest();
		HttpResult 			handleCGI(const std::string& path, const ServerConfig* server, const Location* loc); // magie raciste
		HttpResult			handleGET(std::string& path, const ServerConfig* server, const Location* loc);
		HttpResult			handleHEAD(std::string& path, const ServerConfig* server, const Location* loc);
		HttpResult			handleDELETE(const std::string& path, const ServerConfig* server, const Location* loc);
		HttpResult			handlePOST(const std::string& path, const ServerConfig* server, const Location* loc);
		HttpResult			handleUpload(const std::string& path, const ServerConfig* server, const Location* loc);
		void 				debugRequest(const std::string &file);
		const ServerConfig*	findServer() const;
		std::string			readErrorPage(const ServerConfig& server, int code);
	HttpResult				handleRequestResponse(const ServerConfig* server, int code, const std::string& err, const std::string& path);
		HttpResult			handleAutoindex(const ServerConfig* server, std::string path);

		// Getters Setters
		void				setState(State s);
		State				getState() const;
		int					getFd() const;
};

std::string					getContentType(const std::string &path);
bool						isDirectory(const std::string &path);
std::string					readFile(const std::string& path);
//Goal: To handle the client's connection
