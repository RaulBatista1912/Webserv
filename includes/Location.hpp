#pragma once
#include "Header.hpp"

class Location {
	public:
		std::string	path;
		std::string	root;
		std::string	index;
		std::string	cgiExtension;
		std::string	cgiPath;
		bool		autoindex;
		bool		allowGet;
		bool		allowPost;
		bool		allowDelete;

		Location();
};

