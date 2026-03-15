#pragma once
#include "Header.hpp"

class Response {
	private:
		std::string _reasonPhrase;
		std::map<std::string, std::string> _headers;
		std::string _body;
	public:
		std::string buildResponse(std::string status, std::string body, std::string type);
		Response();
		~Response();
};


