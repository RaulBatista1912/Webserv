#pragma once

#include <vector>
#include <string>
#include "ServerConfig.hpp"

class Config {
    private:
        std::vector<ServerConfig> servers;
        void    ParseFile(const std::string& path);
        void    ParseServerBlock(std::ifstream& file);

    public:
        Config(const std::string& path);
        const std::vector<ServerConfig>& getServers() const;

};