#include "../includes/Client.hpp"
#include "../includes/Session.hpp"
#include "../includes/Utils.hpp"

// extrait caca depuis user=caca&age=42
std::string extractQueryParam(const std::string& query, const std::string& key) {
	size_t pos = query.find(key + "=");

	if (pos == std::string::npos)
		return "";
	pos += key.length() + 1;
	size_t end = query.find("&", pos);
	if (end == std::string::npos)
		end = query.length();

	return query.substr(pos, end - pos);
}

std::string Client::handleRequest(size_t body_len) {
	Response res;
	HttpResult r;

	std::string method = _request.getMethod();
	std::string path = _request.getPath();

	const ServerConfig* server = findServer();
	const Location* loc = server->findLocation(path);

	// 1. LOGOUT : avant initSession, pour ne pas recréer de session
	if (path == "/logout") {
		handleLogout(res, r);
		return res.buildResponse(r);
	}

	// 2. Vérifications globales
	if (_request.getVersion() != "HTTP/1.1") {
		r = res.handleRequestResponse(server, 505, "505 HTTP Version Not Supported");
		return res.buildResponse(r);
	}

	if ((int)body_len > server->max_body_size) {
		r = res.handleRequestResponse(server, 413, "413 Request Entity Too Large");
		return res.buildResponse(r);
	}

	// 3. Init session pour les routes qui en ont besoin
	SessionContext ctx = initSession(res);
	Session& session = *ctx.session;

	// 4. LOGIN : écrit "user" dans la session
	if (path == "/login"){
		r = handleLogin(server, session, method);
		return res.buildResponse(r);
	}

	// 6. PROFILE : lit "user" depuis la session chheck si connecté
	if (path == "/profile") {
		std::string body = "<html><body>";

		if (session._data.find("user") != session._data.end()) {
			std::stringstream ss;
			ss << ++session._visits;
			body += "<h1>Hello " + session._data["user"] + "</h1>";
			body += "<p>Visits: "+ ss.str() +"</p>";
			body += "<a href='/logout'>Logout</a>";
		} else {
			body += "<h1>Not logged in</h1>";
			body += "<a href='/login?user=daniel'>Login</a>";
		}
		body += "</body></html>";
		r.status = "200 OK";
		r.contentType = "text/html";
		r.body = body;
		r.contentLength = r.body.size();

		return res.buildResponse(r);
	}

	// 7. Routing normal
	if (method == "GET")
		r = handleGET(path, server, loc);
	else if (method == "POST")
		r = handlePOST(path, server, loc);
	else if (method == "DELETE")
		r = handleDELETE(path, server, loc);
	else if (method == "HEAD")
		r = handleHEAD(path, server, loc);
	else
		r = res.handleRequestResponse(server, 501, "501 Not Implemented");
	return res.buildResponse(r);
}

HttpResult Client::handleUpload(const std::string& path, const ServerConfig* server, const Location* loc) {
	HttpResult	r;
	Response	res;

	if (!loc->allowPost)
		return res.handleRequestResponse(server, 405, "405 Method Not Allowed");

	std::string contentType = _request.getHeader("Content-Type");
	std::string body = _request.getBody();

	if (body.size() > static_cast<size_t>(server->max_body_size))
		return res.handleRequestResponse(server, 413, "413 Request Entity Too Large");

	// 1) Récupérer le boundary
	size_t pos = contentType.find("boundary=");
	if (pos == std::string::npos)
		return res.handleRequestResponse(server, 400, "400 Bad Request");

	std::string boundary = "--" + contentType.substr(pos + 9);

	// 2) Trouver le début de la première partie
	size_t partStart = body.find(boundary);
	if (partStart == std::string::npos)
		return res.handleRequestResponse(server, 400, "400 Bad Request");
	partStart += boundary.size() + 2; // sauter boundary + CRLF

	// 3) Trouver la fin des headers internes
	size_t headerEnd = body.find("\r\n\r\n", partStart);
	if (headerEnd == std::string::npos)
		return res.handleRequestResponse(server, 400, "400 Bad Request");

	std::string headers = body.substr(partStart, headerEnd - partStart);

	// 4) Extraire le filename
	size_t fn = headers.find("filename=\"");
	if (fn == std::string::npos)
		return res.handleRequestResponse(server, 400, "400 Bad Request");
	fn += 10;
	size_t fnEnd = headers.find("\"", fn);
	std::string filename = headers.substr(fn, fnEnd - fn);

	// 5) Début du fichier
	size_t fileStart = headerEnd + 4;

	// 6) Trouver le boundary final EXACT
	std::string endBoundary = "\r\n" + boundary + "--";
	size_t fileEnd = body.find(endBoundary, fileStart);
	if (fileEnd == std::string::npos)
		return res.handleRequestResponse(server, 400, "400 Bad Request");
	// 7) Extraire les octets du fichier
	std::string fileContent = body.substr(fileStart, fileEnd - fileStart);

	// 8) Construire le chemin final
	std::string filepath = server->root + path + "/" + filename;

	int fd = open(filepath.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0644);
	if (fd < 0)
		return res.handleRequestResponse(server, 404, "404 Not Found");

	write(fd, fileContent.data(), fileContent.size());
	close(fd);

	return res.handleRequestResponse(server, 201, "201 Created");
}

HttpResult Client::handlePOST(const std::string& path, const ServerConfig* server, const Location* loc) {
	Response	res;

	std::string contentType = _request.getHeader("Content-Type");
	if (!loc->allowPost)
		return res.handleRequestResponse(server, 405, "405 Method Not Allowed");
	if (path.find(".cgi") != std::string::npos)
		return handleCGI(path, server, loc);
	// Si c'est un upload → déléguer
	if (contentType.find("multipart/form-data") != std::string::npos)
		return handleUpload(path, server, loc);

	// Sinon POST normal (form-urlencoded ou body simple depuis curl)
	std::string body = _request.getBody();
	std::string value;

	size_t pos = body.find('=');
	if (pos != std::string::npos)
		value = body.substr(pos + 1);
	else
		value = body;
	std::string completePath = server->root + path;
	if (completePath.find("..") != std::string::npos || isDirectory(completePath))
		return res.handleRequestResponse(server, 403, "403 Forbidden");

	std::ofstream out(completePath.c_str(), std::ios::binary);
	if (!out)
		return res.handleRequestResponse(server, 500, "500 Internal Server Error");

	out.write(value.c_str(), value.size());
	out.close();
	return res.handleRequestResponse(server, 201, "201 Created");
}

HttpResult Client::handleDELETE(const std::string& path, const ServerConfig* server, const Location* loc)
{
	Response	res;
	// 1. Vérifier si DELETE est autorisé
	if (loc && !loc->allowDelete)
		return res.handleRequestResponse(server, 405, "405 Method Not Allowed");

	// 2. Construire le chemin réel
	const std::string completePath = server->root + path;

	// 3. Sécurité basique
	if (completePath.find("..") != std::string::npos ||
		completePath.find(".html") != std::string::npos ||
		completePath.find(".js") != std::string::npos ||
		completePath.find(".cpp") != std::string::npos ||
		completePath.find(".hpp") != std::string::npos ||
		isDirectory(completePath))
		return res.handleRequestResponse(server, 403, "403 Forbidden");

	// 5. Vérifier si le fichier existe
	if (0 > access(completePath.c_str(), F_OK | W_OK))
		return res.handleRequestResponse(server, 404, "404 Not Found");

	// 6. Supprimer
	if (std::remove(completePath.c_str()) != 0)
		return res.handleRequestResponse(server, 500, "500 Internal Server Error");

	// 7. Succès
	return res.handleRequestResponse(server, 204, "204 No Content");
}

HttpResult	Client::handleHEAD(std::string& path, const ServerConfig* server, const Location* loc) {
	HttpResult r = handleGET(path, server, loc);
	r.body = "";
	return r;
}


HttpResult	Client::handleGET(std::string& path, const ServerConfig* server, const Location* loc) {
	HttpResult	r;
	Response	res;
	std::string file;

	if (path.find("..") != std::string::npos) {
		r = res.handleRequestResponse(server, 403, "403 Forbidden");
		return r;
	}
	if (loc && !loc->allowGet) {
		r = res.handleRequestResponse(server, 405, "405 Method Not Allowed");
		return r;
	}
	if (loc && loc->redirectCode != 0) {
		HttpResult r;
		r.status = httpStatusToString(loc->redirectCode);
		r.headers["Location"] = loc->redirectPath;
		r.body = "";
		r.contentType = "text/html";
		r.contentLength = 0;
		return r;
	}

	if (path == "/")
		path = "/" + loc->index;
	else if (isDirectory(server->root + path)) {
		if (!loc->index.empty()) {
			if (path[path.length() - 1] != '/')
				path += '/';
			path += loc->index;
		}
		else if (loc->autoindex) {
			r = handleAutoindex(server, path);
			return r;
		}
		else {
			r = res.handleRequestResponse(server, 403, "403 Forbidden");
			return r;
		}
	}
	file = server->root + path;
	if (path.find(".cgi") != std::string::npos)
		return handleCGI(path, server, loc);
	std::ifstream webPage(file.c_str(), std::ios::binary);
	if (webPage) {
		std::stringstream buffer;
		buffer << webPage.rdbuf();
		r.status = "200 OK";
		r.body = buffer.str();
		r.contentType = getContentType(path);
		r.contentLength = r.body.size();
	}
	else
		r = res.handleRequestResponse(server, 404, "404 Not Found");
	return r;
}

HttpResult Client::handleLogin(const ServerConfig* server, Session& session, const std::string& method) {
	HttpResult	r;
	Response	res;

	// GET → afficher formulaire
	if (method == "GET") {
		std::string body =
		"<html><body>"
		"<h1>Login</h1>"
		"<form method='POST' action='/login'>"
		"<input type='text' name='user'/>"
		"<input type='submit' value='Login'/>"
		"</form>"
		"</body></html>";

		r.status = "200 OK";
		r.contentType = "text/html";
		r.body = body;
		r.contentLength = body.size();
		return r;
	}

	// POST → traiter login
	if (method == "POST") {
		std::string body = _request.getBody();
		std::string user = extractQueryParam(body, "user");

		if (!user.empty())
			session._data["user"] = user;

		r.status = "200 OK";
		r.contentType = "text/html";
		r.body = "<html><body>Logged as " + user +
		         "<br><a href='/profile'>Go profile</a></body></html>";
		r.contentLength = r.body.size();
		return r;
	}
	res.handleRequestResponse(server, 405, "405 Method Not Allowed");
	// autre méthode
	r.status = "405 Method Not Allowed";
	r.contentType = "text/plain";
	r.body = "Method Not Allowed\n";
	r.contentLength = r.body.size();
	return r;
}
