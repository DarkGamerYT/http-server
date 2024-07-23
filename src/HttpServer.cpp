#include "HttpServer.h"
inline std::map<std::string, std::string> extractParams(std::string path, std::string regPath, std::string reg);
HttpServer::HttpServer()
{
#if defined(_WIN32)
	WSADATA m_wsaData;
	WSAStartup(MAKEWORD(2, 0), &m_wsaData);
#endif

	this->serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (this->serverSocket < 0)
	{
		std::cout << "Failed to create a socket." << std::endl;
		exit(1);
	};
};
HttpServer::~HttpServer() { this->close(); };

void HttpServer::listen(short port)
{
	this->socketAddress.sin_family = AF_INET;
	this->socketAddress.sin_port = htons(port);
	this->socketAddress.sin_addr.s_addr = INADDR_ANY;
	this->_listen();
};
void HttpServer::listen(const char* address, short port)
{
	this->socketAddress.sin_family = AF_INET;
	this->socketAddress.sin_port = htons(port);
#if defined(_WIN32)
	InetPton(AF_INET, address, &this->socketAddress.sin_addr.s_addr);
#elif defined(__linux__)
	this->socketAddress.sin_addr.s_addr = inet_addr(address);
#endif
	this->_listen();
};
void HttpServer::close()
{
#if defined(_WIN32)
	closesocket(this->serverSocket);
	WSACleanup();
#elif defined(__linux__)
	::close(this->serverSocket);
#endif
};

void HttpServer::_listen()
{
#if defined(__linux__)
	int opt = 1;
	setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif
	if (bind(this->serverSocket, (struct sockaddr*)&socketAddress, sizeof(socketAddress)) < 0)
	{
		std::cout << "Failed to bind address." << std::endl;
#	if defined(_WIN32)
		closesocket(this->serverSocket);
#	elif defined(__linux)
		::close(this->serverSocket);
#	endif
		exit(1);
	};

	if (::listen(this->serverSocket, this->mMaxConnections) < 0)
	{
		std::cout << "Failed to listen." << std::endl;
		exit(1);
	};

	// Receive connections
	while (true)
		std::async([](HttpServer* httpServer) {
		struct sockaddr_in clientAddress;
#	if defined(_WIN32)
		SOCKET clientSocket;
		int addressLength = sizeof(clientAddress);
#	elif defined(__linux__)
		int clientSocket;
		socklen_t addressLength = sizeof(clientAddress);
#	endif
		clientSocket = accept(httpServer->serverSocket, (struct sockaddr*)&clientAddress, &addressLength);
		if (clientSocket < 0)
			return;

		char buffer[65536];
#	if defined(_WIN32)
		int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
#	elif defined(__linux__)
		int bytesReceived = read(clientSocket, buffer, sizeof(buffer));
#	endif
		if (bytesReceived < 1)
			return;

		char* copy = (char*)malloc(strlen(buffer) + 1);
		strcpy(copy, buffer);

		std::string receivedData(copy);
		free(copy);
	
		Request req(receivedData);
		Response res(clientSocket, req);

		std::string requestMethod = req.method();
		std::vector<Route>& routes = httpServer->routes[requestMethod];
		for (const Route& route : routes)
		{
			bool match = std::regex_match(req.route(), std::regex(route.regexp));
			if (true == match)
			{
				std::map<std::string, std::string> params = extractParams(req.route(), route.route, route.regexp);
				req.mParams = params;
				route.handle(req, res);
			};
		};

		if (!res.headersSent())
		{
			if (requestMethod != "GET" &&
				requestMethod != "POST" &&
				requestMethod != "PUT" &&
				requestMethod != "DELETE" &&
				requestMethod != "OPTIONS" &&
				requestMethod != "PATCH" &&
				requestMethod != "HEAD" &&
				requestMethod != "CONNECT" &&
				requestMethod != "TRACE")
			{
				res.setStatus(405);
				res.setHeader("Content-Type", "text/plain");
				res.send("405 - Not Allowed.");
			};
		};
	}, this);
};

inline std::string pathToRegex(std::string route)
{
	std::regex regex(R"([\/,\-@])");
	route = std::regex_replace(route, std::regex(R"(\/:([^\/]+)\?)"), "(?:\\/([^\\/]+))?");
	route = std::regex_replace(route, std::regex(R"(\/:([^\/\?(]+))"), "\\/([^\\/]+)");
	route = std::regex_replace(route, std::regex(R"(\*)"), "(.*)");
	route = std::regex_replace(route, std::regex(R"(\/$)"), "");

	route.append("(?:\\/)?");
	return std::string("^").append(route).append("$");
};

void HttpServer::_use(std::string method, std::string route, std::function<void(Request, Response)> handler)
{
	if (this->routes.find(method) == this->routes.end())
	{
		this->routes[method].push_back({ route, pathToRegex(route), handler });
		return;
	};

	this->routes[method].push_back({ route, pathToRegex(route), handler });
};

inline std::map<std::string, std::string> extractParams(std::string path, std::string regPath, std::string reg)
{
	std::vector<std::string> paramNames;
	std::regex paramRegex(R"(:([^\/\?]+)\??)");
	std::sregex_iterator iter(regPath.begin(), regPath.end(), paramRegex);
	std::sregex_iterator end;
	while (iter != end) {
		paramNames.push_back((*iter)[1]);
		++iter;
	};

	std::smatch match;
	std::regex_match(path, match, std::regex(reg));
	std::map<std::string, std::string> params;
	for (size_t i = 0; i < paramNames.size(); ++i) {
		params[paramNames[i]] = match[i + 1].str();
	};

	return params;
};