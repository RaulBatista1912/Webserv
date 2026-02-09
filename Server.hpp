#pragma once

#include <netinet/in.h>

class Server {
    private:
        int fd;
        int port;
    public:
        Server(int port);
        ~Server();

        int getFd() const;
        int getPort() const;
        int acceptClient() const;
};