void	handleTimeout(std::vector<pollfd> &fds, std::map<int, Client *> &clients);
void	handleSocketServer(Server *currentServer, Config &config, std::vector<pollfd> &fds, std::map<int, Client *> &clients);
void	handleSocketClient(std::map<int, Client *> &clients, std::vector<pollfd> &fds, size_t &i);