#include "../includes/Response.hpp"

Response::Response() {}

Response::~Response() {}

std::string Response::buildResponse(HttpResult r) {
	std::stringstream ss;
	ss << r.contentLength;
	std::string len = ss.str();
	std::string response = "HTTP/1.1 " + r.status + "\r\n"
			"Content-Length: "+ len +"\r\n"
			"Connection: close\r\n"
			"Content-Type: " + r.contentType +"\r\n"
			"\r\n"
			+ r.body;
	//debug
	// std::cout << "----- DEBUG RESPONSE -----" << std::endl;
	// std::cout << response << std::endl;
	// std::cout << "----------END RESPONSE---------------\n" << std::endl;
	return (response);
}
