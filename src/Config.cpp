#include "../includes/Config.hpp"

void	Config::checkListValueDebug(const ServerConfig& srv) {
	std::cout << "Server Name: " << srv.serverName << std::endl;
	std::cout << "Port: " << srv.port << std::endl;
	std::cout << "Root: " << srv.root << std::endl;
	std::cout << std::endl;
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

Config::Config(const std::string& path) {
	ParseFile(path);
	for (size_t i = 0; i < _servers.size(); ++i)
		for (size_t j = 0; j < _servers[i].locations.size(); ++j)
			if (_servers[i].locations[j].path == "/error_page") {
				if (_servers[i].locations[j].allowGet)
					_servers[i].allowErrPage = true;
				else 
					_servers[i].allowErrPage = false;
			}
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

	while (std::getline(file, line)) {
		line = trim(stripComment(line));
		if (line.empty())
			continue;

		if (line == "}")
			break;

		if (line.find("location") == 0) {
			Location loc;
			size_t start = 8; // "location"

			while (start < line.size() && (line[start] == ' ' || line[start] == '\t'))
				++start;

			size_t end = line.find("{", start);
			if (end == std::string::npos)
				end = line.size();

			loc.path = trim(line.substr(start, end - start));

			if (line.find("{", start) == std::string::npos) {
				if (!std::getline(file, line))
					throw std::runtime_error("Error: expected '{' after location " + loc.path);
				line = trim(stripComment(line));
				if (line != "{")
					throw std::runtime_error("Error: expected '{' after location " + loc.path);
			}
			ParseLocationBlock(file, loc);
			server.locations.push_back(loc);
			continue;
		}

		if (line.find("listen") == 0) {
			std::string portStr = trim(line.substr(6));
			int	port;
			std::stringstream ss(portStr);
			ss >> port;
			server.port = port;
		}
		else if (line.find("server_name") == 0) {
			server.serverName = trim(line.substr(11));
		}
		else if (line.find("root") == 0) {
			server.root = removeSemicolon(line.substr(4));
			if (server.root.empty())
				throw std::runtime_error("Root cannot be empty");
			if (server.root == "/")
				throw std::runtime_error("Root '/' is not allowed");
			if (!server.root.empty() && server.root[server.root.size() - 1] == '/')
				server.root.erase(server.root.size() - 1);
		}
		else if (line.find("index") == 0) {
			server.index = removeSemicolon(line.substr(5));
		}
		else if (line.find("client_max_body_size") == 0) {
			std::string MaxBody = trim(line.substr(20));
			int	body;
			std::stringstream ss(MaxBody);
			ss >> body;
			server.max_body_size = body;
		}
		else if (line.find("error_page") == 0) {
			std::istringstream iss(line);
			std::string word;
			int	code;
			std::string path;
			iss >> word >> code >> path;
			if (path.empty()) {
				throw std::runtime_error("Invalid error_page");
			}
			if (!path.empty() && path[path.size() - 1] == ';')
				path = path.substr(0, path.size() - 1);
			server.errorPages[code] = path;
		}
		else {
			throw std::runtime_error("Error: unknown directive in server block: '" + line + "'");
		}
	}
	_servers.push_back(server);
}

// In the Block Location
void Config::ParseLocationBlock(std::ifstream& file, Location& loc) {
	std::string line;

	while (std::getline(file, line)) {
		line = trim(stripComment(line));
		if (line.empty())
			continue;

		if (line == "}")
			break;

		if (line.find("allowed_methods") == 0) {
			std::string methodsStr = trim(line.substr(15));
			std::istringstream iss(methodsStr);
			std::string method;
			while (iss >> method) {
				if (!method.empty() && method[method.size() - 1] == ';')
					method = method.substr(0, method.size() - 1);
				if (method == "GET")
					loc.allowGet = true;
				else if (method == "POST")
					loc.allowPost = true;
				else if (method == "DELETE")
					loc.allowDelete = true;
			}
		}
		else if (line.find("autoindex") == 0) {
			std::string val = trim(line.substr(9));
			loc.autoindex = (val == "on");
		}
		else if (line.find("root") == 0) {
			loc.root = trim(line.substr(4));
		}
		else if (line.find("index") == 0) {
			loc.index = trim(line.substr(5));
		}
		else if (line.find("cgi_extension") == 0) {
			loc.cgiExtension = trim(line.substr(13));
		}
		else if (line.find("cgi_path") == 0) {
			loc.cgiPath = trim(line.substr(8));
		}
		else {
			throw std::runtime_error("Error: unknown directive in location block: '" + line + "'");
		}
	}
}

std::string removeSemicolon(std::string value) {
	value = trim(value);
	if (!value.empty() && value[value.size() - 1] == ';')
		value.erase(value.size() - 1);
	return value;
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

