#include <vector>
#include <map>
#include <iostream>
#include <cstdio>
#include <poll.h>
#include "../includes/Client.hpp"
#include "../includes/Server.hpp"
#include "../includes/Config.hpp"

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

		// creer un serveur pour chaque port,
		for (size_t i = 0; i < serverConfigs.size(); ++i) {
			Server* srv = new Server(serverConfigs[i].port, serverConfigs[i].root,
				serverConfigs[i].index);
			servers.push_back(srv);

			pollfd p;
			p.fd = srv->getFd();
			p.events = POLLIN; // on demande a poll de surveiller POLLIN, en gros dis moi si des clients veulent se connecter
			p.revents = 0;
			fds.push_back(p);

			std::cout << "Listening on port " << serverConfigs[i].port << std::endl; // on afficher que le serv est pret sur ce port
		}
		while (true) {
			// polling
			if (poll(&fds[0], fds.size(), -1) < 0)
				throw std::runtime_error("poll failed");
			// identifie chaque fd du tableau fds pour savoir si on a un socket serveur ou socket client
			// si serveur -> gerer accept() pour un nouveau client
			// si client -> gerer lecture/ecriture
			for (size_t i = 0; i < fds.size(); ++i) {
				bool isServer = false;
				Server* currentServer = NULL;

				for (size_t s = 0; s < servers.size(); ++s) {
					if (fds[i].fd == servers[s]->getFd()) { // si egal, fd est le socket serveur, donc socket qui peut acceper des connexions
						isServer = true;
						currentServer = servers[s];
						break;
					}
				}
				// socket serveur, nouveau client qui arrive, on accepte
				if (isServer && (fds[i].revents & POLLIN)) {
					int clientFd = currentServer->acceptClient(); // on accepte
					if (clientFd >= 0) {
						Client* c = new Client(clientFd, config);
						clients[clientFd] = c;

						pollfd p;
						p.fd = clientFd;
						p.events = POLLIN;
						p.revents = 0;
						fds.push_back(p);
					}
				}
				// socket client, lire ou ecrire avec le client
				else if (fds[i].revents & (POLLIN | POLLOUT)) { // vrai si soit POLLIN, soit POLLOUT, soit les deux
					Client* c = clients[fds[i].fd];
					// lecture, si erreur -> on met en CLOSED
					if ((fds[i].revents & POLLIN) && !c->readFromSocket())// est-ce que la lecture du socket s'est bien passée ?
						c->setState(Client::CLOSED);
					// ecriture, si erreur -> on met en CLOSED
					if ((fds[i].revents & POLLOUT) && !c->writeToSocket())
						c->setState(Client::CLOSED);
					// si le client ferme ou erreur
					if (c->getState() == Client::CLOSED) {
						close(c->getFd());
						delete c;
						clients.erase(fds[i].fd);
						fds.erase(fds.begin() + i);
						--i;
						continue;
					}
					fds[i].events = 0;
					// si le client est en READING, on dit a poll de surveiller l'arrivee de donnes POLLIN
					if (c->getState() == Client::READING)
						fds[i].events |= POLLIN;
					// si le serveur doit envoyer des donnes au client, on dit a poll de surveiller quand le socket est pret a ecrire
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
// Goal:
// lire config
// ↓
// créer les serveurs
// ↓
// attendre des événements réseau avec poll()
// ↓
// nouveau client → accept()
// ↓
// client envoie requête → recv()
// ↓
// serveur répond → send()
