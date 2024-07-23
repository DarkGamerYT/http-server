#pragma once
#if defined(_WIN32)
#pragma comment(lib, "Ws2_32.lib")
#include <WinSock2.h>
#include <ws2tcpip.h>
#elif defined(__linux__)
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

#include <regex>
#include <map>
#include <future>
#include <iostream>
#include <sstream>
#include <string.h>

#include "Request.h"
#include "Response.h"
struct Route
{
	std::string route;
	std::string regexp;
	std::function<void(Request, Response)> handle;
};

class HttpServer
{
private:
	SocketType serverSocket;
	sockaddr_in socketAddress;
	std::map<std::string, std::vector<Route>> routes;

protected:
	int mMaxConnections = 20;

public:
	HttpServer();
	~HttpServer();

	void get(std::string route, std::function<void(Request, Response)> handler)
	{ this->_use("GET", route, handler); };
	void post(std::string route, std::function<void(Request, Response)> handler)
	{ this->_use("POST", route, handler); };
	void put(std::string route, std::function<void(Request, Response)> handler)
	{ this->_use("PUT", route, handler); };
	void del(std::string route, std::function<void(Request, Response)> handler)
	{ this->_use("DELETE", route, handler); };
	void options(std::string route, std::function<void(Request, Response)> handler)
	{ this->_use("OPTIONS", route, handler); };
	void patch(std::string route, std::function<void(Request, Response)> handler)
	{ this->_use("PATCH", route, handler); };
	void head(std::string route, std::function<void(Request, Response)> handler)
	{ this->_use("HEAD", route, handler); };
	void connect(std::string route, std::function<void(Request, Response)> handler)
	{ this->_use("CONNECT", route, handler); };
	void trace(std::string route, std::function<void(Request, Response)> handler)
	{ this->_use("TRACE", route, handler); };

	void listen(short port);
	void listen(const char* address, short port);
	void close();

private:
	void _use(std::string method, std::string route, std::function<void(Request, Response)> handler);
	void _listen();
};