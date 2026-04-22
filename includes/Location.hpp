#pragma once
#include "Header.hpp"

class Location {
	public:
		std::string	path;
		std::string	root;
		std::string	index;
		std::string	uploadPath;
		std::string	cgiExtension;
		std::string	cgiPath;
		std::string redirectPath;	// ex: "/new/path"
		int 		redirectCode;	// ex: 301

		bool		autoindex;
		bool		allowGet;
		bool		allowPost;
		bool		allowDelete;

		Location() : redirectCode(0), autoindex(false), allowGet(false), allowPost(false), allowDelete(false) {}
};

