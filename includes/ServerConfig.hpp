#pragma once
#include "Header.hpp"
#include "Location.hpp"

class ServerConfig {
    public:
        int         port;
        std::string serverName;
        std::string root;
        std::string index;
		int	max_body_size;
		std::map<int, std::string> errorPages;

		std::vector<Location> locations;

		ServerConfig() : port(-1),
						serverName("undefined"),
						root("undefined"),
						index("undefined"),
						max_body_size(-1) {}
};