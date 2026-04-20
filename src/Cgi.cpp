#include "../includes/Client.hpp"

extern char **environ;  //Utiliser pour transmettre au CGI les var d'environ

 // Le CGI ecrit ses propres headers HTTP sur stdout.
 static std::string findCgiContentType(const std::string& cgiOutput) {
 	size_t headersEnd = cgiOutput.find("\r\n\r\n");
 	if (headersEnd == std::string::npos)
 		return "";
 	std::string headers = cgiOutput.substr(0, headersEnd);
 	std::string key = "Content-Type:";
 	size_t pos = headers.find(key);
 	if (pos == std::string::npos)
 		return "";
 	pos += key.size();
 	while (pos < headers.size() && (headers[pos] == ' ' || headers[pos] == '\t'))
 		++pos;
 	size_t end = headers.find("\r\n", pos);
 	if (end == std::string::npos)
 		end = headers.size();
 	return headers.substr(pos, end - pos);
 }

 HttpResult Client::handleCGI(const std::string& path, const ServerConfig* server, const Location* loc)
 {
 	(void)loc;
 	HttpResult r;
 	std::string fullPath = server->root + path;
 	std::string requestBody = _request.getBody();

	int outPipe[2];
	int inPipe[2];
	if (pipe(outPipe) == -1) {
		r.status = "500 Internal Server Error";
		r.body = "pipe error";
		return r;
	}
	if (pipe(inPipe) == -1) {
		close(outPipe[0]);
		close(outPipe[1]);
 		r.status = "500 Internal Server Error";
 		r.body = "pipe error";
 		return r;
 	}

 	pid_t pid = fork();
	if (pid < 0) {
		close(outPipe[0]);
		close(outPipe[1]);
		close(inPipe[0]);
		close(inPipe[1]);
		r.status = "500 Internal Server Error";
		r.body = "fork error";
		return r;
	}

 	if (pid == 0) {

 		//Redirige stdout du CGI vers le pipe pour que le parent recupere la sortie.
		if (dup2(outPipe[1], STDOUT_FILENO) == -1)
			_exit(1);
 		//Redirige stdin du CGI pour recevoir le body des requetes POST.
		if (dup2(inPipe[0], STDIN_FILENO) == -1)
			_exit(1);

 		close(outPipe[0]);
 		close(outPipe[1]);
 		close(inPipe[0]);
 		close(inPipe[1]);

 		std::stringstream len;
 		len << requestBody.size();
 		std::string contentLength = len.str();

 		//Variables CGI utilisees par le binaire
 		setenv("QUERY_STRING", _queryString.c_str(), 1);
 		setenv("REQUEST_METHOD", _request.getMethod().c_str(), 1);
 		setenv("CONTENT_LENGTH", contentLength.c_str(), 1);
 		setenv("CONTENT_TYPE", _request.getHeader("Content-Type").c_str(), 1);
 		setenv("SCRIPT_FILENAME", fullPath.c_str(), 1);

 		char *argv[] = {(char *)fullPath.c_str(), NULL};
 		execve(fullPath.c_str(), argv, environ);
		_exit(1);
 	}
 	else {
 		close(outPipe[1]);
 		close(inPipe[0]);

		if (!requestBody.empty()) {
			size_t totalWritten = 0;
			while (totalWritten < requestBody.size()) {
				ssize_t w = write(inPipe[1], requestBody.c_str() + totalWritten,
					requestBody.size() - totalWritten);
				if (w > 0)
					totalWritten += static_cast<size_t>(w);
				else if (w < 0 && errno == EINTR)
					continue;
				else
					break;
			}
		}

 		close(inPipe[1]);

 		char buffer[4096];
 		std::string output;
		ssize_t bytes = 0;

		while ((bytes = read(outPipe[0], buffer, sizeof(buffer))) > 0)
 			output.append(buffer, bytes);

		close(outPipe[0]);
 		waitpid(pid, NULL, 0);

 		r.status = "200 OK";
 		size_t headersEnd = output.find("\r\n\r\n");
 		// On retire les headers CGI pour ne garder que le body HTTP final.
 		if (headersEnd != std::string::npos)
 			r.body = output.substr(headersEnd + 4);
 		else
 			r.body = output;
 		r.contentType = findCgiContentType(output);
 		if (r.contentType.empty())
 			r.contentType = "text/plain";
		r.contentLength = r.body.size();
 		std::cout << "[CGI] query='" << _queryString << "' body='" << r.body << "'" << std::endl;
 		return r;
 	}
 }