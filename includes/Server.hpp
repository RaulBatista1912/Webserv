#pragma once
#include "Header.hpp"

class Server {
    private:
        int _fd;// server's socket
        int _port; // port where the server listens for incoming connections
    public:
        Server(int port);
        ~Server();

        int getFd() const;
        int getPort() const;
        int acceptClient() const;
};
//Goal: To listen the incoming connections