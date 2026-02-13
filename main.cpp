#include <vector>
#include <map>
#include <iostream>
#include <cstdio>
#include <poll.h>
#include "Client.hpp"
#include "Server.hpp"
#include "Config.hpp"

int main(int ac, char** av)
{
    if (ac != 2) {
        std::cerr << "Usage: ./webserv <config_file>\n";
        return 1;
    }
    try {
        Config config(av[1]); 
        const std::vector<ServerConfig>& serverConfigs = config.getServers();
        std::vector<Server*> servers;
        std::vector<pollfd> fds;
        std::map<int, Client*> clients;

        for (size_t i = 0; i < serverConfigs.size(); ++i) {
            Server* srv = new Server(serverConfigs[i].port);
            servers.push_back(srv);

            pollfd p;
            p.fd = srv->getFd();
            p.events = POLLIN;
            p.revents = 0;
            fds.push_back(p);

            std::cout << "Listening on port " << serverConfigs[i].port << std::endl;
        }
        while (true) {
            if (poll(&fds[0], fds.size(), -1) < 0)
                throw std::runtime_error("poll failed");

            for (size_t i = 0; i < fds.size(); ++i) {
                bool isServer = false;
                Server* currentServer = NULL;

                for (size_t s = 0; s < servers.size(); ++s) {
                    if (fds[i].fd == servers[s]->getFd()) {
                        isServer = true;
                        currentServer = servers[s];
                        break;
                    }
                }
                if (isServer && (fds[i].revents & POLLIN)) {
                    int clientFd = currentServer->acceptClient();
                    if (clientFd >= 0) {
                        Client* c = new Client(clientFd);
                        clients[clientFd] = c;

                        pollfd p;
                        p.fd = clientFd;
                        p.events = POLLIN;
                        p.revents = 0;
                        fds.push_back(p);
                        std::cout << "New client fd=" << clientFd << std::endl;
                    }
                }
                else if (fds[i].revents & (POLLIN | POLLOUT)) {
                    Client* c = clients[fds[i].fd];
                    if ((fds[i].revents & POLLIN) && !c->readFromSocket())
                        c->setState(Client::CLOSED);
                    if ((fds[i].revents & POLLOUT) && !c->writeToSocket())
                        c->setState(Client::CLOSED);
                    if (c->getState() == Client::CLOSED) {
                        close(c->getFd());
                        delete c;
                        clients.erase(fds[i].fd);
                        fds.erase(fds.begin() + i);
                        --i;
                        continue;
                    }
                    fds[i].events = 0;
                    if (c->getState() == Client::READING)
                        fds[i].events |= POLLIN;
                    if (c->getState() == Client::WRITING)
                        fds[i].events |= POLLOUT;
                }
            }
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 0;
}
