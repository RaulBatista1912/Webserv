#include "../includes/Header.hpp"

std::string readFile(const std::string& path) {
	std::ifstream file(path.c_str());
	std::stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}

bool isDirectory(const std::string &path) {
	struct stat st;
	if (stat(path.c_str(), &st) == -1)
		return false;
	return S_ISDIR(st.st_mode);
}

std::string httpStatusToString(int code) {
	if (code == 301) return "301 Moved Permanently";
	if (code == 302) return "302 Found";
	if (code == 307) return "307 Temporary Redirect";
	if (code == 308) return "308 Permanent Redirect";
	return "302 Found";
}

std::string getContentType(const std::string &path)
{
	if (path.find(".html") != std::string::npos)
		return "text/html";
	if (path.find(".css") != std::string::npos)
		return "text/css";
	if (path.find(".js") != std::string::npos)
		return "application/javascript";
	if (path.find(".png") != std::string::npos)
		return "image/png";
	if (path.find(".jpg") != std::string::npos)
		return "image/jpeg";
	if (path.find(".gif") != std::string::npos)
		return "image/gif";
	return "text/html";
}