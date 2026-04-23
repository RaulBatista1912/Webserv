#pragma once
#include "Header.hpp"

class Request {
	private:
		std::string _method;
		std::string _path;
		std::string _version;
		std::map<std::string, std::string> _headers;
		std::map<std::string, std::string> _cookies;
		std::string _body;

	public:
		Request();
		~Request();

		// Public methods
		bool parse(const std::string& Request);
		void parseCookies();

		std::map<std::string, std::string>
			extractHeaders(const std::string &rawHeader);

		// Getters Setters
		const std::string& getMethod() const								{return _method;}
		const std::string& getPath() const									{return _path;}
		const std::string& getVersion() const								{return _version;}
		const std::string& getBody() const									{return _body;}
		const std::map<std::string, std::string>& getHeaders() const		{return _headers;}
		const std::string& getHeader(const std::string& contentType) const;
		const std::string getCookie(const std::string& name) const;
};
