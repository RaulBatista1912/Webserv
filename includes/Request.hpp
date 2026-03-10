#pragma once

#include <string>
#include <map>

class Request {
    private:
        std::string _method;
        std::string _path;
        std::string _version;
        std::map<std::string, std::string> _headers;
        std::string _body;

    public:
        Request();
        ~Request();

        bool parse(const std::string& Request);

        const std::string& getMethod() const;
        const std::string& getPath() const;
        const std::string& getVersion() const;
        const std::map<std::string, std::string>& getHeaders() const;
};