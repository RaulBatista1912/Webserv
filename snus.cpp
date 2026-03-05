
// Verifie les sockets dans fds, attends qu'un evenement arrive comme POLLIN, POLLOUT ou POLLERR/POLLHUP
// si un client envoie des donnes ou si le serveur a une connexion -> poll remplit revents, return le nombre de sockets actifs(osef pour nous)
// si erreur -> return -1
int poll(struct pollfd *fds, nfds_t nfds, int timeout); 
// struct pollfd, nombre d'elements, -1 pour indefiniment(ce qu'on veut)

struct pollfd {
    int fd;        // descripteur de fichier
    short events;  // événements à surveiller
    short revents; // événements détectés
};

|=  // ajout de bits, par exemple : fds[i].events = POLLIN
    //                              fds[i].event |= POLLOUT
    //                              fds[i].events = POLLIN | POLLOUT

//creation du socket
fd = socket(AF_INET, SOCK_STREAM, 0);
// AF_INET == socket IPv4(format de l'ip, par exemple 127.0.0.1)
// SOCK_STREAM == TCP(protocol de transport assurant une comminication fiable utiliser pour navigation web, transert de fichier etc)
//             != UDP(protocol de transfert rapide, privilegie la vitesse a la fiablilite, utilise pour les jeux, le streaming etc)
// 0 == protocole par defaut

// set options on socket
int opt = 1;
setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
// fd == le socket
// SOL_SOCKET == option pour le socket lui meme
// SO_REUSEADDR == utiliser le serveur sur le meme port 
// &opt, sizeof(opt) == valeur 1 pour activer l'option, 0 pour desactiver, la fonction attend un pointeur vers la valeur

if (bind(fd, (sockaddr*)&addr, sizeof(addr)) < 0)
// associe le socket a une adresse IP et port precis

sockaddr_in addr; // struct IPv4
std::memset(&addr, 0, sizeof(addr)); // initialise a 0
addr.sin_family = AF_INET; // IPv4
addr.sin_addr.s_addr = INADDR_ANY; // accepte toutes les interfaces reseau
addr.sin_port = htons(port); // definit port, htons() converti host -> network byte order

htons() // == Host TO Network Short, converti un entier de 16bits(port) de l'ordre de octets du pc host vers l'ordre standard du reseau(big-endian) pour que tous les systemes TCP/IP comprennent le port

listen(fd, 10) // transforme le socket en mode passif, il devient pret a accpeter des clients, le serveur peut attendre jusqu'a 10 clients ici, ensuite on pourra les accept()

// accept(fd, NULL, NULL) suffit pour nous, osef de l'address et la taille de la struct
int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen); // socket serveur en ecoute, ptr vers struct qui recoit l'addresse du client, ptr vers un entier qui a la taille de la struct addr 