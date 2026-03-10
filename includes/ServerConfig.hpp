#pragma once
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstdlib>
#include <stdexcept>
#include <cctype>

class ServerConfig {
    public:
        int         port;
        std::string serverName;
        std::string host;
        std::string root;
        std::string index;

        ServerConfig() : port(-1),
                        serverName(""),
                        host(""),
                        root(""),
                        index("") {}
};