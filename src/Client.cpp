#include "../includes/Client.hpp"
#include "../includes/Utils.hpp"

Client::Client(int fd, Config& config, Server* server):
_fd(fd), _state(READING), _config(config), _server(server), _timeout(std::time(NULL)) {}

Client::~Client() {
	if (_fd >= 0)
		close(_fd);
}

// Parse la request-line brute
static std::string extractQueryFromRequestLine(const std::string& rawRequest) {
	size_t lineEnd = rawRequest.find("\r\n");
	if (lineEnd == std::string::npos)
		return "";
	std::string requestLine = rawRequest.substr(0, lineEnd);

	size_t firstSpace = requestLine.find(' ');
	if (firstSpace == std::string::npos)
		return "";
	size_t secondSpace = requestLine.find(' ', firstSpace + 1);
	if (secondSpace == std::string::npos)
		return "";

	std::string target = requestLine.substr(firstSpace + 1, secondSpace - firstSpace - 1);
	size_t qPos = target.find('?');
	if (qPos == std::string::npos)
		return "";
	return target.substr(qPos + 1);
}

const ServerConfig*	Client::findServer() const {
	const std::vector<ServerConfig>& servers = _config.getServers();
	const std::map<std::string, std::string>& headers = _request.getHeaders();
	std::map<std::string, std::string>::const_iterator it = headers.find("Host");

	if (it == headers.end() || it->second.empty())
		return &servers[0];

	std::string host = it->second;
	std::size_t pos = host.find(':');
	int	port = 0;
	if (pos != std::string::npos) {
		std::string portStr = host.substr(pos + 1);
		port = std::atoi(portStr.c_str());
	}

	for (size_t i = 0; i < servers.size(); ++i) {
		if (port == servers[i].port)
			return &servers[i];
	}
	std::cout << std::endl;
	return &servers[0];
}

HttpResult	Client::handleAutoindex(const ServerConfig* server, std::string& path) {
	HttpResult	r;
	Response	res;
	std::stringstream html;

	// Construire le chemin réel sur le disque
	if (path[path.size() - 1] != '/')
		path += '/';
	std::string realPath = server->root + path;

	// Début du HTML
	html << "<html><head><title>Index of " << path << "</title></head><body>";
	html << "<h1>Index of " << path << "</h1><ul>";

	DIR* dir = opendir(realPath.c_str());
	if (!dir) {
		return res.handleRequestResponse(server, 500, "500 Internal Server Error");
	}

	struct dirent* entry;
	while ((entry = readdir(dir)) != NULL) {
		std::string name = entry->d_name;

		if (name == "." || name == "..")
			continue;

		// Toujours forcer un slash final
		std::string href = path;
		if (href.empty() || href[href.size() - 1] != '/')
			href += '/';

		href += name;

		html << "<li><a href=\"" << href << "\">" << name << "</a></li>";
	}
	closedir(dir);
	html << "</ul></body></html>";

	// Construire la réponse HTTP
	r.status = "200 OK";
	r.body = html.str();
	r.contentType = "text/html";
	r.contentLength = r.body.size();

	return r;
}

bool	Client::readFromSocket() {
	char	buffer[4096];
	int	bytes = recv(_fd, buffer, sizeof(buffer), 0);

	if (bytes <= 0)
		return false;
	_readBuffer.append(buffer, bytes);
	size_t header_end = _readBuffer.find("\r\n\r\n");
	if (header_end == std::string::npos)
		return true;
	size_t body_len = 0;
	std::string headers = _readBuffer.substr(0, header_end + 4);
	if (header_end == std::string::npos)
		return true;
	// ensuite seulement parser les headers
	std::map<std::string, std::string> h = _request.extractHeaders(headers);
	if (h.count("Content-Length"))
		body_len = std::atoi(h["Content-Length"].c_str());
	if (_readBuffer.size() < header_end + 4 + body_len)
		return true;
	// Sauvegarde la query brute avant _request.parse(), qui supprime '?...'.
	_queryString = extractQueryFromRequestLine(_readBuffer);
	if (!_request.parse(_readBuffer)) {
		Response res;
		HttpResult r;
		r.status = "400 Bad Request";
		r.body = "<h1>400 Bad Request</h1>";
		r.contentType = "text/html";
		_writeBuffer = res.buildResponse(r);
		_state = WRITING;
		return true;
	}
	_request.parseCookies();
	_writeBuffer = handleRequest(body_len);
	_state = WRITING;
	return true;
}

bool	Client::writeToSocket() {
	if (_writeBuffer.empty()) {
		_state = CLOSED;
		return (false);
	}
	ssize_t sent = send(_fd, _writeBuffer.c_str(), _writeBuffer.size(), 0);
	if (sent <= 0) {
		_state = CLOSED;
		return (false);
	}
	_writeBuffer.erase(0, sent);
	if (_writeBuffer.empty())
		_state = CLOSED;
	return (true);
}

//get the session user or create it if doesn't exist
SessionContext Client::initSession(Response& res) {
	SessionContext ctx;

	SessionManager& sm = _server->getSessionManager();
	std::string sid = _request.getCookie("session_id");
	ctx.session = &sm.getOrCreateSession(sid, ctx.created);
	if (ctx.created) {
		res.addSetCookie(buildSessionCookie(ctx.session->_id, 3600));
	}
	return ctx;
}

HttpResult Client::handleLogin(const ServerConfig* server, Session& session, const std::string& method)
{
	HttpResult r;
	Response res;

	// =========================
	// GET → afficher page login
	// =========================
	if (method == "GET" || method == "HEAD")
	{
		// déjà connecté → redirect
		if (session._data.find("user") != session._data.end())
		{
			r.status = "302 Found";
			r.headers["Location"] = "/profile";
			r.body = "";
			r.contentType = "text/html";
			r.contentLength = 0;
			return r;
		}
		std::string file = server->root + "/auth/login.html";
		std::ifstream f(file.c_str());
		if (!f)
			return res.handleRequestResponse(server, 404, "404 Not Found");
		std::stringstream buffer;
		buffer << f.rdbuf();
		std::string content = buffer.str();
		r.status = "200 OK";
		r.contentType = "text/html";
		r.contentLength = r.body.size();
		if (method == "GET")
			r.body = content;
		else if (method == "HEAD")
			r.body = "";
		r.contentLength = content.size();
		return r;
	}

	// =========================
	// POST → traiter login
	// =========================
	if (method == "POST")
	{
		std::string body = _request.getBody();
		std::string user = extractQueryParam(body, "user");

		// validation
		if (user.empty())
		{
			r.status = "400 Bad Request";
			r.contentType = "text/html";
			r.body = "<html><body><h1>400 Bad Request</h1><p>Missing username</p></body></html>";
			r.contentLength = r.body.size();
			return r;
		}

		// enregistrer session
		session._data["user"] = user;

		// redirect vers profile
		r.status = "302 Found";
		r.headers["Location"] = "/profile";
		r.body = "";
		r.contentType = "text/html";
		r.contentLength = 0;
		return r;
	}

	// =========================
	// AUTRES MÉTHODES → 405
	// =========================
	return res.handleRequestResponse(server, 405, "405 Method Not Allowed");
}

HttpResult Client::handleProfile(const ServerConfig* server, Session& session, const std::string& method)
{
	HttpResult r;
	Response res;

	if (method == "GET" || method == "HEAD")
	{
		std::string file = server->root + "/auth/profile.html";
		std::ifstream f(file.c_str());
		if (!f)
			return res.handleRequestResponse(server, 404, "404 Not Found");
		std::stringstream buffer;
		buffer << f.rdbuf();
		std::string body = buffer.str();
		if (session._data.find("user") != session._data.end()) {
			std::string user = session._data["user"];

			std::stringstream vs;
			vs << ++session._visits;

			// remplacer placeholders
			size_t pos;
			pos = body.find("{{USER}}");
			if (pos != std::string::npos)
				body.replace(pos, 8, user);
			pos = body.find("{{VISITS}}");
			if (pos != std::string::npos)
				body.replace(pos, 10, vs.str());
		}
		else {
			body = "<h1>Not logged in</h1><a href='/login'>Login</a>";
		}
		r.status = "200 OK";
		r.contentType = "text/html";
		if (method == "GET")
			r.body = body;
		else if(method == "HEAD")
			r.body = "";
		r.contentLength = body.size();
		return r;
	}
	if (method == "POST" || method == "DELETE")
		return res.handleRequestResponse(server, 405, "405 Method Not Allowed");
	return res.handleRequestResponse(server, 501, "501 Not Implemented");
}

HttpResult Client::handleLogout(const ServerConfig* server, const std::string& method) {
	HttpResult r;
	Response res;

	if (method == "GET" || method == "HEAD")
	{
		SessionManager& sm = _server->getSessionManager();
		std::string sid = _request.getCookie("session_id");

		if (!sid.empty())
			sm.deleteSession(sid);
		res.addSetCookie("session_id=deleted; Path=/; Max-Age=0; HttpOnly");
		std::string file = server->root + "/auth/logout.html";
		std::ifstream f(file.c_str());
		if (!f)
			return res.handleRequestResponse(server, 404, "404 Not Found");
		std::stringstream buffer;
		buffer << f.rdbuf();
		std::string body = buffer.str();
		r.status = "200 OK";
		r.contentType = "text/html";
		if (method == "GET")
			r.body = body;
		else if (method == "HEAD")
			r.body = "";
		r.contentLength = body.size();
		return r;
	}
	return res.handleRequestResponse(server, 405, "405 Method Not Allowed");
}

void	Client::debugRequest(const std::string &file) {
	std::cout << "----- DEBUG REQUEST -----" << std::endl;
	std::cout << "New client fd=" << _fd << std::endl;
	std::cout << "Method:  " << _request.getMethod() << std::endl;
	std::cout << "Path:    " << _request.getPath() << std::endl;
	std::cout << "Version: " << _request.getVersion() << std::endl;
	std::cout << "\nHeaders:" << std::endl;
	std::map<std::string, std::string> headers = _request.getHeaders();
	for (std::map<std::string, std::string>::iterator it = headers.begin();
		it != headers.end(); ++it)
	{
		std::cout << "  " << it->first << ": " << it->second << std::endl;
	}
	std::cout << "\nBody:" << std::endl;
	std::cout << _request.getBody() << std::endl;
	std::cout << "\nServer is searching: " << file << std::endl;
	std::cout << "----------END REQUEST---------------\n" << std::endl;
}
