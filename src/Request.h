#pragma once
#include <map>
#include <iostream>
#include <sstream>
#include <string.h>

#include "String.hpp"
class HttpServer;
class Request
{
	friend HttpServer;

private:
	std::string mMethod, mRoute, mBody;
	std::map<std::string, std::string> mParams, mHeaders;

public:
	Request(std::string data);

	std::string route() const { return this->mRoute; };
	std::string method() const { return this->mMethod; };
	std::string body() const { return this->mBody; };
	std::map<std::string, std::string> params() const { return this->mParams; };
	std::map<std::string, std::string> headers() const { return this->mHeaders; };
};