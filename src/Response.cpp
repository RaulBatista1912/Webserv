#include "../includes/Response.hpp"

Response::Response() {}

Response::~Response() {}

std::string Response::buildResponse(std::string status, std::string body, std::string type) {
	std::stringstream ss;
	ss << body.size();
	std::string len = ss.str();
	std::string response = "HTTP/1.1 " + status +"\r\n"
			"Content-Length: "+ len +"\r\n"
			"Connection: close\r\n"
			"Content-Type: " + type +"\r\n"
			"\r\n"
			+ body;
	//debug
	//std::cout << response << std::endl;
	return (response);
}
