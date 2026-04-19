#pragma once
#include "Header.hpp"
#include "Location.hpp"

class ServerConfig {
	public:
		int							port;
		std::string					serverName;
		std::string					root;
		int							max_body_size;
		bool						allowErrPage;
		std::map<int, std::string>	errorPages;

		std::vector<Location>		locations;

		ServerConfig() : port(-1),
						serverName("undefined"),
						root("undefined"),
						max_body_size(0) {}
		const Location* findLocation(const std::string& path) const;
};