#include "../includes/Header.hpp"
#include "../includes/Utils.hpp"
#include "../includes/Client.hpp"

void	handleTimeout(std::vector<pollfd> &fds, std::map<int, Client *> &clients) {
	time_t	timeout = std::time(NULL);
	for (size_t i = 0; i < fds.size(); ++i) {
		if (clients.find(fds[i].fd) == clients.end())
			continue;
		Client* c = clients[fds[i].fd];
		if (c->getState() != Client::CLOSED && timeout - c->getTime() > 15) {
			Response	res;
			HttpResult	r;
			r.status = "408 Request Timeout";
			r.body = "<h1>408 Request Timeout</h1>";
			r.contentType = "text/html";
			r.contentLength = r.body.size();
			c->setWriteBuffer(res.buildResponse(r));
			c->setState(Client::WRITING);
			fds[i].events |= POLLOUT;
		}
	}
}

void	handleSocketServer(Server *currentServer, Config &config, std::vector<pollfd> &fds, std::map<int, Client *> &clients) {
	while (true) {
		int clientFd = currentServer->acceptClient(); // on accepte
		if (clientFd < 0) {
			if (isTemporaryAcceptError(errno))
				break;
			break;
		}
		Client* c = new Client(clientFd, config, currentServer);
		clients[clientFd] = c;

		pollfd p;
		p.fd = clientFd;
		p.events = POLLIN;
		p.revents = 0;
		fds.push_back(p);
	}
}

void	handleSocketClient(std::map<int, Client *> &clients, std::vector<pollfd> &fds, size_t &i) {
	Client* c = clients[fds[i].fd];
	if (fds[i].revents & (POLLERR | POLLHUP | POLLNVAL)) {
		c->setState(Client::CLOSED);
	}
	// lecture, si erreur -> on met en CLOSED
	if ((fds[i].revents & POLLIN) && !c->readFromSocket())// est-ce que la lecture du socket s'est bien passée ?
		c->setState(Client::CLOSED);
	else if (fds[i].revents & POLLIN)
		c->setTime(time(NULL));
	// ecriture, si erreur -> on met en CLOSED
	if ((fds[i].revents & POLLOUT) && !c->writeToSocket())
		c->setState(Client::CLOSED);
	else if (fds[i].revents & POLLOUT)
		c->setTime(time(NULL));
	// si le client ferme ou erreur
	if (c->getState() == Client::CLOSED) {
		close(c->getFd());
		delete c;
		clients.erase(fds[i].fd);
		fds.erase(fds.begin() + i);
		--i;
		return;
	}
	fds[i].events = 0;
	// si le client est en READING, on dit a poll de surveiller l'arrivee de donnes POLLIN
	if (c->getState() == Client::READING)
		fds[i].events |= POLLIN;
	// si le serveur doit envoyer des donnes au client, on dit a poll de surveiller quand le socket est pret a ecrire
	if (c->getState() == Client::WRITING)
		fds[i].events |= POLLOUT;
}

void	CreateServers(Config &config, std::vector<Server*> &servers, std::vector<pollfd> &fds) {
	const std::vector<ServerConfig>& serverConfigs = config.getServers();

		// creer un serveur pour chaque port,
		for (size_t i = 0; i < serverConfigs.size(); ++i) {
			Server* srv = new Server(serverConfigs[i].port, serverConfigs[i].root);
			servers.push_back(srv);
			pollfd p;
			p.fd = srv->getFd();
			p.events = POLLIN; // on demande a poll de surveiller POLLIN, en gros dis moi si des clients veulent se connecter
			p.revents = 0;
			fds.push_back(p);
			std::cout << "Listening on port " << serverConfigs[i].port << std::endl; // on afficher que le serv est pret sur ce port
		}
}