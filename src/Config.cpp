#include "../includes/Config.hpp"

void	Config::checkListValueDebug(const ServerConfig& srv) {
	std::cout << std::endl << "Server Name: " << srv.serverName << std::endl <<
		"Host: " << srv.host << std::endl << "Port: " << srv.port << std::endl 
		<< "Root: " << srv.root << std::endl << std::endl;
}

// Supprime les espaces et tabs au début et à la fin de la str
static std::string trim(const std::string& value) {
	std::string::size_type start = 0;
	while (start < value.size() && std::isspace(static_cast<unsigned char>(value[start])))
		++start;

	std::string::size_type end = value.size();
	while (end > start && std::isspace(static_cast<unsigned char>(value[end - 1])))
		--end;

	return value.substr(start, end - start);
}

//Skip les comms
static std::string stripComment(const std::string& value) {
	std::string::size_type commentPos = value.find('#');
	if (commentPos != std::string::npos)
		return value.substr(0, commentPos);
	return value;
}

//isDigits mais le version 2
static bool isDigits(const std::string& value) {
	if (value.empty())
		return false;
	for (std::string::size_type i = 0; i < value.size(); ++i) {
		if (!std::isdigit(static_cast<unsigned char>(value[i])))
			return false;
	}
	return true;
}

//check porc
static int parsePort(const std::string& value) {
	if (!isDigits(value))
		throw std::runtime_error("Error: listen port needs to be a number");
	std::stringstream ss(value);
	int port = -1;
	ss >> port;
	if (ss.fail() || !ss.eof() || port < 1024 || port > 65535)
		throw std::runtime_error("Error: invalid listen port");
	return port;
}

// recois le nom et l args;
static std::string parseSingleValue(const std::string& directiveLine, const std::string& keyName) {
	std::stringstream stream(directiveLine);
	std::string readKey;
	std::string value;

	stream >> readKey;
	stream >> value;
	if (readKey.empty())
		throw std::runtime_error("Error: lines is empty");
	else if (value.empty())
		throw std::runtime_error("Error: " + keyName + " block requires a value");

	//check si y a pas de troisième mot ou une snus.
	std::string extra;
	if (stream >> extra)
		throw std::runtime_error("Error: invalid " + keyName + " invalid syntax");

	return value;
}

Config::Config(const std::string& path) {
	ParseFile(path);
}

const std::vector<ServerConfig>& Config::getServers() const {
	return (_servers);
}

//premiere boucle Main qui lit le fichier conf
void Config::ParseFile(const std::string& path) {
	std::ifstream file(path.c_str());
	if (!file.is_open()) {
		throw std::runtime_error("Error: cannot open config file");
	}
	else if (path.size() < 5 || path.substr(path.size() - 5) != ".conf")
		throw std::runtime_error("Error: config file must have .conf extension");

	std::string line;
	bool waitingServerBody = false;
	while (std::getline(file, line)) {
		//Nettoyage
		line = trim(stripComment(line));
		if (line.empty())
			continue;

		if (line == "server") {
			waitingServerBody = true; 
			continue; // server found on continue
		}

		if ((line == "server {") || (waitingServerBody && line == "{")) {
			waitingServerBody = false;

			ParseServerBlock(file); // Parsing par block {}
			continue;
		}
		throw std::runtime_error("Error: invalid top-level directive: '" + line + "'");
	}

	if (waitingServerBody)
		throw std::runtime_error("Error: expected '{' after server");

	if (_servers.empty()) {
		throw std::runtime_error("Error: no server block found");
	}
}

// Parse le contenu d'un bloc server { ... } ligne par ligne.
// Crée un objet ServerConfig local, remplit ses champs au fur et à mesure,
// puis l'ajoute dans `_servers` une fois le bloc fermé par '}'.
void Config::ParseServerBlock(std::ifstream& file) {
	ServerConfig server;
	std::string line;
	bool listenFound = false;
	bool serverNameFound = false;
	bool hostFound = false;
	bool rootFound = false;
	bool indexFound = false;

	while (std::getline(file, line)) {
		line = trim(stripComment(line));
		if (line.empty())
			continue;

		if (line == "}")
			break;

		if (line[line.size() - 1] != ';')
			throw std::runtime_error("Error: missing ';' in server block: '" + line + "'");

		line = trim(line.substr(0, line.size() - 1));
		std::stringstream ss(line);
		std::string key;
		ss >> key;
		if (key == "listen") {
			if (listenFound)
				throw std::runtime_error("Error: duplicate listen directive in server block");

			std::stringstream listenStream(line);
			std::string listenKeyword;
			std::string listenValue;
			listenStream >> listenKeyword;
			listenStream >> listenValue;

			//check
			if (listenValue.empty())
				throw std::runtime_error("Error: listen directive requires a port");
			//extra si il y a encore du text apres
			std::string extra;
			if (listenStream >> extra)
				throw std::runtime_error("Error: invalid listen directive syntax");

			//port
			server.port = parsePort(listenValue);
			listenFound = true;
		}
		else if (key == "server_name") {
			if (serverNameFound)
				throw std::runtime_error("Error: duplicate server_name directive in server block");
			//serverName
			server.serverName = parseSingleValue(line, "server_name");
			serverNameFound = true;
		}
		else if (key == "host") {
			if (hostFound)
				throw std::runtime_error("Error: duplicate host directive in server block");
			//`server.host`
			server.host = parseSingleValue(line, "host");
			hostFound = true;
		}
		else if (key == "root") {
			if (rootFound)
				throw std::runtime_error("Error: duplicate root directive in server block");
			//`server.root`
			server.root = parseSingleValue(line, "root");
			rootFound = true;
		}
		else if (key == "index") {
			if (indexFound)
				throw std::runtime_error("Error: duplicate index directive in server block");
			//`server.index`
			server.index = parseSingleValue(line, "index");
			indexFound = true;
		}
		else {
			throw std::runtime_error("Error: unknown directive in server block: '" + key + "'");
		}
	}


/*___________________________________*/
	   //Check list de Variables

	   //TODO stocker les fuckings regle
	   //TODO voir pour un switch case trop de if else

	if (!listenFound)
		throw std::runtime_error("Error: missing listen directive in server block");
	if (!serverNameFound)
		throw std::runtime_error("Error: missing server_name directive in server block");
	if (!hostFound)
		throw std::runtime_error("Error: missing host directive in server block");
	if (server.port <= 0)
		throw std::runtime_error("Error: invalid listen port");

	if (file.eof() && line != "}") {
		throw std::runtime_error("Error: missing closing '}' for server block");
	}
	checkListValueDebug(server);
	_servers.push_back(server);
}

/*
                 *       +
           '                  |
       ()    .-.,="``"=.    - o -
             '=/_       \     |
          *   |  '=._    |
               \     `=./`,        '
            .   '=.__.=' `='      *
   +                         +
        O      *        '       .
*/

