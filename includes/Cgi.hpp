#include <string>
#include "../includes/Client.hpp"

static std::string	findCgiContentType(const std::string& cgiOutput);
HttpResult			handleCGI(const std::string& path, const ServerConfig* server, const Location* loc);