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
    sockaddr_in addr; // struct IPv4
    std::memset(&addr, 0, sizeof(addr)); // initialise a 0
    addr.sin_family = AF_INET; // IPv4
    addr.sin_addr.s_addr = INADDR_ANY; // accepte toutes les interfaces reseau
    addr.sin_port = htons(port); // definit port, htons() converti host -> network byte order

    if (bind(fd, (sockaddr*)&addr, sizeof(addr)) < 0) // associe le socket serveur a l'addresse IP + port
        throw std::runtime_error("bind failed");
    if (listen(fd, 10) < 0) // met le socket TCP en mode passif pour accepet des connexions et creer une gile d'attente des clients en attente
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