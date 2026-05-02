#include "../includes/Header.hpp"
#include "../includes/Client.hpp"
#include "../includes/Server.hpp"
#include "../includes/Config.hpp"
#include "../includes/Poll.hpp"
#include "../includes/Utils.hpp"

int g_running = 1;

void handleSignal(int sig) {
	(void)sig;
	g_running = 0;
}

void	free_all(std::vector<Server*> servers, std::map<int, Client *> clients, std::vector<pollfd> fds) {
	for (size_t i = 0; i < servers.size(); i++)
			delete (servers[i]);
	for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ++it) {
		close(it->first);
		delete (it->second);
	}
	servers.clear();
	clients.clear();
	fds.clear();
}

int main(int ac, char** av) {
	if (ac != 2) {
		std::cerr << "Usage: ./webserv <config_file>\n";
		return 1;
	}
	std::signal(SIGINT, handleSignal);
	std::signal(SIGTERM, handleSignal);
	std::vector<Server*> servers;
	std::map<int, Client*> clients;
	std::vector<pollfd> fds;
	time_t lastCleanup;
	try {
		Config config(av[1]);
		CreateServers(config, servers, fds);
		lastCleanup = std::time(NULL);
		while (g_running) {
			// polling blocks until an event appears
			int	ret = poll(&fds[0], fds.size(), 1000); //update each 1s
			if (ret < 0) {
				if (errno == EINTR)
					continue;
				throw std::runtime_error("poll failed");
			}
			handleTimeout(fds, clients);
			time_t	now = std::time(NULL);
			if (now - lastCleanup >= 10) { // toutes les 10s
				for (size_t s = 0; s < servers.size(); ++s)
					servers[s]->getSessionManager().cleanupExpired();
				lastCleanup = now;
			}
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
				if (isServer && (fds[i].revents & POLLIN))
					handleSocketServer(currentServer, config, fds, clients);
				// socket client, lire ou ecrire avec le client
				else if (fds[i].revents & (POLLIN | POLLOUT | POLLERR | POLLHUP | POLLNVAL)) // vrai si on a un des flags
					handleSocketClient(clients, fds, i);
			}
		}
	}
	catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
	}
	free_all(servers, clients, fds);
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
