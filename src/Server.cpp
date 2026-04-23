#include "../includes/Server.hpp"

Server::Server(int port, const std::string& root): _fd(-1), _port(port), _root(root) {
	_fd = socket(AF_INET, SOCK_STREAM, 0); // AF_INET = IPv4, SOCK_STREAM = protocol TCP, 0 = protocol TCP par default
	if (_fd < 0)
		throw std::runtime_error("socket failed");

	int opt = 1;
	setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	if (fcntl(_fd, F_SETFL, O_NONBLOCK) < 0)
		throw std::runtime_error("fcntl failed");

	sockaddr_in add_p4;
	std::memset(&add_p4, 0, sizeof(add_p4));
	add_p4.sin_family = AF_INET; 				// IPv4
	add_p4.sin_addr.s_addr = INADDR_ANY; 		// accepte toutes les interfaces reseau, inet_addr("127.0.0.1") pour seulement localhost
	add_p4.sin_port = htons(port); 				// definit port, htons() converti host -> network byte order

	add_p4.sin_family = AF_INET; 				// IPv4
	add_p4.sin_addr.s_addr = INADDR_ANY; 		// accepte toutes les interfaces reseau
	add_p4.sin_port = htons(port); 				// definit port, htons() converti host -> network byte order

	//	CHECK ADD_P4
	if (bind(_fd, (sockaddr*)&add_p4, sizeof(add_p4)) < 0) // associe le socket serveur a l'addresse IP + port
		throw std::runtime_error("bind failed");
	if (listen(_fd, SOMAXCONN) < 0) 					  // met le socket TCP en mode passif pour accepet des connexions et creer une gile d'attente des clients en attente
		throw std::runtime_error("listen failed");
}

Server::~Server() {
	if (_fd >= 0)
		close(_fd);
}