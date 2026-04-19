#pragma once
#include "Header.hpp"

std::string					httpStatusToString(int code);
std::string					getContentType(const std::string &path);
bool						isDirectory(const std::string &path);
std::string					readFile(const std::string& path);