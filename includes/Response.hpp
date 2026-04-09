#pragma once
#include "Header.hpp"
#include "Client.hpp"

class Response {
	private:
		std::string _reasonPhrase;
		std::map<std::string, std::string> _headers;
		std::vector<std::string> setCookies;
		std::string _body;
	public:
		std::string buildResponse(HttpResult r);
		Response();
		~Response();
};


