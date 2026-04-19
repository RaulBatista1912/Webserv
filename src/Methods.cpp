#include "../includes/Header.hpp"

std::string	Client::handleRequest(size_t body_len) {
	Response res;
	HttpResult r;
	std::string method = _request.getMethod();
	//std::cout << method << std::endl;
	std::string path = _request.getPath();
	//std::cout << path << std::endl;
	const ServerConfig* server = findServer();
	//std::cout << server->port << std::endl;
	const Location* loc = server->findLocation(path);
	//std::cout << loc << std::endl;

	//debug poce bleu
	//debugRequest(server->root + path);

	if ((int)body_len > server->max_body_size)
		r = res.handleRequestResponse(server, 413, "413 Request Entity Too Large", path);
	else if (method == "GET")
		r = handleGET(path, server, loc);
	else if (method == "POST")
		r = handlePOST(path, server, loc);
	else if (method == "DELETE")
		r = handleDELETE(path, server, loc);
	else if (method == "HEAD")
		r = handleHEAD(path, server, loc);
	else
		r = res.handleRequestResponse(server, 501, "501 Not Implemented", path);
	return res.buildResponse(r);
}

HttpResult Client::handleUpload(const std::string& path, const ServerConfig* server, const Location* loc) {
	HttpResult	r;
	Response	res;

	if (!loc->allowPost)
		return res.handleRequestResponse(server, 405, "405 Method Not Allowed", path);

	std::string contentType = _request.getHeader("Content-Type");
	std::string body = _request.getBody();

	if (body.size() > static_cast<size_t>(server->max_body_size))
		return res.handleRequestResponse(server, 413, "413 Request Entity Too Large", path);

	// 1) Récupérer le boundary
	size_t pos = contentType.find("boundary=");
	if (pos == std::string::npos)
		return res.handleRequestResponse(server, 400, "400 Bad Request", path);

	std::string boundary = "--" + contentType.substr(pos + 9);

	// 2) Trouver le début de la première partie
	size_t partStart = body.find(boundary);
	if (partStart == std::string::npos)
		return res.handleRequestResponse(server, 400, "400 Bad Request", path);
	partStart += boundary.size() + 2; // sauter boundary + CRLF

	// 3) Trouver la fin des headers internes
	size_t headerEnd = body.find("\r\n\r\n", partStart);
	if (headerEnd == std::string::npos)
		return res.handleRequestResponse(server, 400, "400 Bad Request", path);

	std::string headers = body.substr(partStart, headerEnd - partStart);

	// 4) Extraire le filename
	size_t fn = headers.find("filename=\"");
	if (fn == std::string::npos)
		return res.handleRequestResponse(server, 400, "400 Bad Request", path);
	fn += 10;
	size_t fnEnd = headers.find("\"", fn);
	std::string filename = headers.substr(fn, fnEnd - fn);

	// 5) Début du fichier
	size_t fileStart = headerEnd + 4;

	// 6) Trouver le boundary final EXACT
	std::string endBoundary = "\r\n" + boundary + "--";
	size_t fileEnd = body.find(endBoundary, fileStart);
	if (fileEnd == std::string::npos)
		return res.handleRequestResponse(server, 400, "400 Bad Request", path);
	// 7) Extraire les octets du fichier
	std::string fileContent = body.substr(fileStart, fileEnd - fileStart);

	// 8) Construire le chemin final
	std::string filepath = server->root + path + "/" + filename;

	int fd = open(filepath.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0644);
	if (fd < 0)
		return res.handleRequestResponse(server, 404, "404 Not Found", path);

	write(fd, fileContent.data(), fileContent.size());
	close(fd);

	return res.handleRequestResponse(server, 201, "201 Created", path);
}

HttpResult Client::handlePOST(const std::string& path, const ServerConfig* server, const Location* loc) {
	Response	res;

	std::string contentType = _request.getHeader("Content-Type");
	if (!loc->allowPost)
		return res.handleRequestResponse(server, 405, "405 Method Not Allowed", path);
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
		return res.handleRequestResponse(server, 403, "403 Forbidden", path);

	std::ofstream out(completePath.c_str(), std::ios::binary);
	if (!out)
		return res.handleRequestResponse(server, 500, "500 Internal Server Error", path);

	out.write(value.c_str(), value.size());
	out.close();
	return res.handleRequestResponse(server, 201, "201 Created", path);
}

HttpResult Client::handleDELETE(const std::string& path, const ServerConfig* server, const Location* loc)
{
	Response	res;
	// 1. Vérifier si DELETE est autorisé
	if (loc && !loc->allowDelete)
		return res.handleRequestResponse(server, 405, "405 Method Not Allowed", path);

	// 2. Construire le chemin réel
	const std::string completePath = server->root + path;

	// 3. Sécurité basique
	if (completePath.find("..") != std::string::npos && isDirectory(completePath))
		return res.handleRequestResponse(server, 403, "403 Forbidden", path);

	// 5. Vérifier si le fichier existe
	if (0 < access(completePath.c_str(), F_OK) || 0 < access(completePath.c_str(), W_OK))
		return res.handleRequestResponse(server, 404, "404 Not Found", path);

	// 6. Supprimer
	if (std::remove(completePath.c_str()) != 0)
		return res.handleRequestResponse(server, 500, "500 Internal Server Error", path);

	// 7. Succès
	return res.handleRequestResponse(server, 204, "204 No Content", path);
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
		r = res.handleRequestResponse(server, 403, "403 Forbidden", path);
		return r;
	}
	if (loc && !loc->allowGet) {
		r = res.handleRequestResponse(server, 405, "405 Method Not Allowed", path);
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
			r = res.handleRequestResponse(server, 403, "403 Forbidden", path);
			return r;
		}
	}
	file = server->root + path;
	if (path.find(".cgi") != std::string::npos)
		return handleCGI(path, server, loc);
	file = server->root + path;
	std::ifstream webPage(file.c_str(), std::ios::binary);
	if (webPage) {
		//std::cout << "OPEN FILE: " << file << "\n" << std::endl;
		std::stringstream buffer;
		buffer << webPage.rdbuf();
		r.status = "200 OK";
		r.body = buffer.str();
		r.contentType = getContentType(path);
		r.contentLength = r.body.size();
	}
	else
		r = res.handleRequestResponse(server, 404, "404 Not Found", path);
	return r;
}
