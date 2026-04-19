#pragma once
#include "Header.hpp"
#include "Client.hpp"

struct HttpResult;

class Response {
	private:
		std::string _reasonPhrase;
		std::map<std::string, std::string> _headers;
		std::vector<std::string> _setCookies;
		std::string _body;
	public:
		Response();
		~Response();
		std::string	buildResponse(HttpResult r);
		HttpResult	handleRequestResponse(const ServerConfig* server, int code, const std::string& status, const std::string& path);
		std::string	readErrorPage(const ServerConfig& server, int code);
		void addSetCookie(const std::string& cookieLine);
};
std::string buildSessionCookie(const std::string& sessionId, int maxAge);
