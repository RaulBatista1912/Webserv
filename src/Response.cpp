#include "../includes/Response.hpp"
#include "../includes/Utils.hpp"

std::string	Response::buildResponse(HttpResult r) {
	std::stringstream ss;
	ss << r.contentLength;
	std::string len = ss.str();
	std::string response = "HTTP/1.1 " + r.status + "\r\n"
			"Location: "+ r.headers["Location"] +"\r\n"
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

std::string	Response::readErrorPage(const ServerConfig& server, int code) {
	std::map<int, std::string>::const_iterator it = server.errorPages.find(code);

	if (it != server.errorPages.end()) {
		std::string filePath = server.root + "/" + it->second;

		std::ifstream file(filePath.c_str(), std::ios::binary);
		if (file) {
			std::stringstream buffer;
			buffer << file.rdbuf();
			return buffer.str();
		}
	}
	return "<h1>Error</h1>";
}

HttpResult	Response::handleRequestResponse(const ServerConfig* server, int code, const std::string& status, const std::string& path) {
	HttpResult r;

	if (server->allowErrPage && server->errorPages.find(code) != server->errorPages.end())
		r.body = readErrorPage(*server, code);
	else
		r.body = "<h1>" + status + "<h1>";
	r.status = status;
	r.contentLength = r.body.size();
	r.contentType = getContentType(path);
	return r;
}

Response::Response() {}

Response::~Response() {}
