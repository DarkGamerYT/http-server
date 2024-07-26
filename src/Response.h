#pragma once
#if defined(_WIN32)
#include <WinSock2.h>
#elif defined(__linux__)
#include <unistd.h>
#endif

#include <filesystem>
#include <iostream>
#include <map>
#include <fstream>
#include <sstream>
#include <string.h>

#include "Request.h"
#include "HttpStatus.h"
#if defined(_WIN32)
#define GETSOCKETERRNO() (WSAGetLastError())
#else
#define GETSOCKETERRNO() (errno)
#endif

#if defined(_WIN32)
typedef SOCKET SocketType;
#elif defined(__linux__)
typedef int SocketType;
#endif

class HttpServer;
class Response
{
	friend HttpServer;
private:
	int statusCode = 200;
	bool mHeadersSent = false;
	std::map<std::string, std::string> headers;

protected:
	SocketType mClientSocket;
	Request mRequest;

public:
	Response(SocketType clientSocket, Request req) :
		mClientSocket(clientSocket),
		mRequest(req) {};

	bool headersSent() const { return this->mHeadersSent; };
	void setStatus(int status) { this->statusCode = status; };

	bool redirect(std::string location);
	bool redirect(std::string location, int status);
	bool send(std::string data);
	bool sendFile(std::string path);

	void setHeader(std::string key, std::string value) { this->headers[key] = value; };
	std::string getHeader(std::string key) { return this->headers[key]; };
	bool removeHeader(std::string key) { return this->headers.erase(key) != 0; };
private:
	std::ostringstream _write();
	bool _send(std::string data);
	void _closeSocket();
};