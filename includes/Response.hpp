#pragma once
#include "Header.hpp"

class Response {
	private:
		//int _statusCode;
		std::string _reasonPhrase;
		std::map<std::string, std::string> _headers;
		std::string _body;
	public:
		std::string buildResponse(std::string body);
		Response();
		~Response();
};


