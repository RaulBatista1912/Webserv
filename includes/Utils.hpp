#pragma once
#include "Header.hpp"

std::string					httpStatusToString(int code);
std::string					getContentType(const std::string &path);
bool						isDirectory(const std::string &path);
std::string					readFile(const std::string& path);
bool						isTemporaryAcceptError(int err);
std::string 				extractQueryParam(const std::string& query, const std::string& key);
bool						isValidUsername(const std::string& user);
std::string					trim(const std::string& s);
std::string					urlDecode(const std::string& str);
