#include "../includes/Request.hpp"
#include <sstream>
#include <iostream>

Request::Request() {
}

Request::~Request() {
}

bool Request::parse(const std::string& Request) {
    std::stringstream ss(Request);
    std::string line;

    if (!std::getline(ss, line))
        return false;
    if (line[line.size() - 1] == '\r')
        line.erase(line.size() - 1);

    std::stringstream firstLine(line);

    if (!(firstLine >> _method >> _path >> _version))
        return false;

    size_t queryPos = _path.find('?');
    if (queryPos != std::string::npos)
        _path = _path.substr(0, queryPos);
    // else
    //     _path = _path;

    while(std::getline(ss, line)) {
        if (line == "\r" || line == "")
            break;
        if (line[line.size() - 1] == '\r')
            line.erase(line.size() - 1);

        size_t pos = line.find(":");
        if (pos == std::string::npos)
            continue;
        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);

        if (!value.empty() && value[0] == ' ')
            value.erase(0, 1);
        _headers[key] = value;
    }
    std::string body;
    std::getline(ss, body, '\0');
    _body = body;
    return true;
}

const std::string& Request::getMethod() const {
    return _method;
}

const std::string& Request::getPath() const {
    return _path;
}

const std::string& Request::getVersion() const {
    return _version;
}

const std::string& Request::getBody() const {
    return _body;
}

const std::map<std::string, std::string>& Request::getHeaders() const {
    return _headers;
}