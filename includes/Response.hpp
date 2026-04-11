#pragma once
#include "Header.hpp"
#include "Client.hpp"

class Response {
	private:
		std::string _reasonPhrase;
		std::map<std::string, std::string> _headers;
		std::string _body;
	public:
		std::string	buildResponse(HttpResult r);
		HttpResult	handleRequestResponse(const ServerConfig* server, int code, const std::string& status, const std::string& path);
		std::string	readErrorPage(const ServerConfig& server, int code);
		Response();
		~Response();
};


