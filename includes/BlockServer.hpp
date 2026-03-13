#pragma once
#include "Header.hpp"
#include "ServerConfig.hpp"

class ServerConfig;

class BlockServer
{
private:
    // bool    GET;
    // bool    PUT;     SINON CA COMPILE PAS
    // bool    POST;
    std::string cbbs;
    std::string root;
    std::string alias;
    std::string cgip;
    std::string index;
        // client_body_buffer_size
        // root
        // alias
        // cgi_pass
        // index
public:
    BlockServer();
    BlockServer(const ServerConfig& obj, std::ifstream& file);
    ~BlockServer();
};

