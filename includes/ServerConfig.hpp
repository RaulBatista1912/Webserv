#pragma once
#include "Header.hpp"
#include "BlockServer.hpp"

class BlockServer;

class ServerConfig {
    public:
        int         port;
        std::string serverName;
        std::string host;
        std::string root;
        std::string index;
        bool        GET;
        bool        PUT;
        bool        POST;
        BlockServer *bs;

        ServerConfig() : port(-1),
                        serverName("undefined"),
                        host("undefined"),
                        root("undefined"),
                        index("undefined"),
                        GET(false),
                        PUT(false),
                        POST(false),
                        bs() {}
};