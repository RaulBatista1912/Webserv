#include "../includes/Response.hpp"

Response::Response()
{
}

Response::~Response()
{
}

std::string Response::buildResponse(std::string body)
{
	std::string len = std::to_string(body.size());
	std::string response = "HTTP/1.1 200 OK\r\n"
			"Content-Length: "+ len +"\r\n"
			"Connection: close\r\n"
			"\r\n"
			+ body;
	return (response);
}