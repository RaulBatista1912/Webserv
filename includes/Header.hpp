#pragma once

#include <string> //func string
#include <vector> // list
#include <fstream> //read opin fill
#include <map> //list
#include <iostream> // cout
#include <sstream>
#include <netinet/in.h>
#include <cstdlib>
#include <stdexcept> //exception
#include <cctype>
#include <unistd.h> // C library
#include <sys/socket.h> //chaussette
#include <fcntl.h>
#include <dirent.h>   // opendir, readdir, closedir
#include <sys/stat.h> // stat
#include <sys/types.h>
#include <sys/wait.h> //time
#include <cerrno> //ERRNO

#include "Client.hpp"
#include "Config.hpp"
#include "Location.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "Server.hpp"
#include "ServerConfig.hpp"
#include "Session.hpp"
#include "Utils.hpp"
