#include "../includes/Config.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstdlib>

Config::Config(const std::string& path) {
    ParseFile(path);
}

const   std::vector<ServerConfig>& Config::getServers() const {
    return (servers);
}

void    Config::ParseFile(const std::string& path) {
    std::ifstream file(path.c_str());
    if (!file.is_open()) {
        std::cerr << "Error: cannot open config file\n";
        std::exit(1);
    }
    std::string line;
    while (std::getline(file, line)) {
        if (line.find("server") != std::string::npos) {
            ParseServerBlock(file);
        }
    }
    if (servers.empty()) {
        std::cerr << "Error: no server block found\n";
        std::exit(1);
    }
}

void    Config::ParseServerBlock(std::ifstream& file) {
    ServerConfig server;
    std::string line;

    while (std::getline(file, line)) {
        if (line.find("listen") != std::string::npos) {
            std::stringstream ss(line);
            std::string word;
            ss >> word;
            ss >> word;
            server.port = std::atoi(word.c_str());
        }
        if (line.find("}") != std::string::npos)
            break;
    }
    if (server.port <= 0) {
        std::cerr << "Error: invalid listen port\n";
        std::exit(1);
    }
    servers.push_back(server);
}

