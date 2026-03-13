#include "../includes/BlockServer.hpp"

 BlockServer::BlockServer() : //GET(false), 
//                             PUT(false),  SINON CA COMPILE PAS
//                             POST(false),
                            cbbs("undefined"),
                            root("undefined"),
                            alias("undefined"),
                            cgip("undefined"),
                            index("undefined") {}

BlockServer::BlockServer(const ServerConfig& srv, std::ifstream& file) {
    //kaks
    (void)srv;
    (void)file;
}

BlockServer::~BlockServer() {}