#include "../includes/Header.hpp"

Request::Request() {
}

Request::~Request() {
}

bool Request::parse(const std::string& raw) {
	size_t header_end = raw.find("\r\n\r\n");
	if (header_end == std::string::npos)
		return false;

	// 1. Parse request line + headers (OK avec stringstream)
	std::stringstream ss(raw.substr(0, header_end));
	std::string line;

	if (!std::getline(ss, line))
		return false;
	if (!line.empty() && line[line.size() - 1] == '\r')
		line.erase(line.size() - 1);

	std::stringstream firstLine(line);
	if (!(firstLine >> _method >> _path >> _version))
		return false;

	size_t queryPos = _path.find('?');
	if (queryPos != std::string::npos)
		_path = _path.substr(0, queryPos);

	while (std::getline(ss, line)) {
		if (line == "\r" || line == "")
			break;
		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);

		size_t pos = line.find(":");
		if (pos == std::string::npos)
			continue;

		std::string key = line.substr(0, pos);
		std::string value = line.substr(pos + 1);

		if (!value.empty() && value[0] == ' ')
			value.erase(0, 1);

		_headers[key] = value;
	}

	// 2. EXTRAIRE LE BODY CORRECTEMENT (LE FIX 🔥)
	size_t content_length = 0;
	if (_headers.count("Content-Length"))
		content_length = std::atoi(_headers["Content-Length"].c_str());

	_body = raw.substr(header_end + 4, content_length);

	return true;
}

std::map<std::string, std::string>
Request::extractHeaders(const std::string &rawHeader)
{
	std::map<std::string, std::string> headers;
	std::istringstream stream(rawHeader);
	std::string line;

	// Ignorer la première ligne (GET / HTTP/1.1)
	std::getline(stream, line);

	while (std::getline(stream, line)) {
		if (line == "\r" || line.empty())
			break;

		// enlever le \r final
		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);

		size_t pos = line.find(':');
		if (pos == std::string::npos)
			continue;
		std::string key = line.substr(0, pos);
		std::string value = line.substr(pos + 1);

		// trim espaces au début
		while (!value.empty() && (value[0] == ' ' || value[0] == '\t'))
			value.erase(0, 1);

		headers[key] = value;
	}
	return headers;
}

const std::string& Request::getMethod() const {
	return _method;
}

const std::string& Request::getPath() const {
	return _path;
}

const std::string& Request::getVersion() const {
	return _version;
}

const std::string& Request::getBody() const {
	return _body;
}

const std::string& Request::getHeader(const std::string& key) const {
	std::map<std::string, std::string>::const_iterator it = _headers.find(key);
	if (it != _headers.end())
		return it->second;

	static const std::string empty = "";
	return empty;
}


const std::map<std::string, std::string>& Request::getHeaders() const {
	return _headers;
}
