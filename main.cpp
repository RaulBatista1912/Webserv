#include <vector>
#include <map>
#include <iostream>
#include <cstdio>
#include <poll.h>
#include "Client.hpp"
#include "Server.hpp"

int main()
{
    try {
        Server server(8080);
        std::cout << "Server listening on port " << server.getPort() << std::endl;
        std::vector<pollfd> fds;
        std::map<int, Client*> clients;

        pollfd serverPoll;
        serverPoll.fd = server.getFd();
        serverPoll.events = POLLIN;
        serverPoll.revents = 0;
        fds.push_back(serverPoll);

        while (true) {
            if (poll(&fds[0], fds.size(), -1) < 0)
                break;
            for (size_t i = 0; i < fds.size(); ++i) {
                if (fds[i].fd == server.getFd() && (fds[i].revents & POLLIN)) {
                    int clientFd = server.acceptClient();
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
        std::cerr << "Fatal error: " << e.what() << std::endl;
    }
    return (0);
}
