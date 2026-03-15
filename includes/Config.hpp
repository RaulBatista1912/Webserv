#pragma once
#include "Header.hpp"
#include "ServerConfig.hpp"
#include "Location.hpp"

class Config {
	private:
		std::vector<ServerConfig>	_servers;
		void	ParseFile(const std::string& path);
		void	ParseServerBlock(std::ifstream& file);
		void	ParseLocationBlock(std::ifstream& file, Location& loc);

	public:
		Config(const std::string& path);

		void	checkListValueDebug(const ServerConfig& srv);
		const	std::vector<ServerConfig>& getServers() const;
};
std::string removeSemicolon(std::string value);