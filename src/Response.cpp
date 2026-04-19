#include "../includes/Response.hpp"
#include "../includes/Utils.hpp"

Response::Response() {}

Response::~Response() {}

std::string Response::buildResponse(HttpResult r) {
	std::stringstream ss;
	ss << r.contentLength;

	std::string response = "HTTP/1.1 " + r.status + "\r\n";

	// Server
	response += "Server: webserv/1.0\r\n";

	// Date (format HTTP)
	time_t now = time(NULL);
	char dateBuf[128];
	struct tm *gmt = gmtime(&now);
	strftime(dateBuf, sizeof(dateBuf), "%a, %d %b %Y %H:%M:%S GMT", gmt);
	response += std::string("Date: ") + dateBuf + "\r\n";

	// Location (uniquement si présent)
	if (r.headers.count("Location") && !r.headers["Location"].empty()) {
		response += "Location: " + r.headers["Location"] + "\r\n";
	}

	// Content-Length + Content-Type
	response += "Content-Length: " + ss.str() + "\r\n";
	response += "Content-Type: " + r.contentType + "\r\n";

	// Connection
	if (r.headers.count("Connection"))
		response += "Connection: " + r.headers["Connection"] + "\r\n";
	else
		response += "Connection: close\r\n";

	// Autres headers
	for (std::map<std::string, std::string>::iterator it = r.headers.begin();
		 it != r.headers.end(); ++it)
	{
		if (it->first != "Location" && it->first != "Connection")
			response += it->first + ": " + it->second + "\r\n";
	}
	for (size_t i = 0; i < _setCookies.size(); ++i) {
		response += "Set-Cookie: " + _setCookies[i] + "\r\n";
	}

	// Fin des headers
	response += "\r\n";

	// Body
	response += r.body;

	return response;
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

void Response::addSetCookie(const std::string& cookieLine) {
	_setCookies.push_back(cookieLine);
}

std::string buildSessionCookie(const std::string& sessionId, int maxAge) {
	std::ostringstream oss;
	oss << "session_id=" << sessionId
		<< "; Path=/"
		<< "; HttpOnly"
		<< "; SameSite=Lax";
	if (maxAge > 0)
		oss << "; Max-Age=" << maxAge;
	return oss.str();
}