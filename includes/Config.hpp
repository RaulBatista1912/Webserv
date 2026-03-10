#pragma once

#include <vector>
#include <string>
#include <fstream>
#include "ServerConfig.hpp"

class Config {
    private:
        std::vector<ServerConfig> _servers;
        void    ParseFile(const std::string& path);
        void    ParseServerBlock(std::ifstream& file);

    public:
        Config(const std::string& path);

        void  checkListValueDebug(const ServerConfig& srv);
        const std::vector<ServerConfig>& getServers() const;
};