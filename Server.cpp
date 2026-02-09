#include "Server.hpp"
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <stdexcept>

Server::Server(int port) : fd(-1), port(port) {
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
        throw std::runtime_error("socket failed");
    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(fd, (sockaddr*)&addr, sizeof(addr)) < 0)
        throw std::runtime_error("bind failed");
    if (listen(fd, 10) < 0)
        throw std::runtime_error("listen failed");
}

Server::~Server() {
    if (fd >= 0)
        close(fd);
}

int Server::getFd() const {
    return (fd);
}

int Server::getPort() const {
    return (port);
}

int Server::acceptClient() const {
    return (accept(fd, NULL, NULL));
}