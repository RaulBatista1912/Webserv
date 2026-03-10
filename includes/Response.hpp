#pragma once
#include <iostream>
#include <map>

class Response
{
private:
	int statusCode;
	std::string reasonPhrase;
	std::map<std::string, std::string> headers;
	std::string body;
public:
	std::string buildResponse(std::string body);
};


