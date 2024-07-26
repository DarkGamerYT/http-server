#include "HttpServer.h"
HttpServer::HttpServer()
{
	this->useSSL = false;
#if defined(_WIN32)
	WSADATA m_wsaData;
	WSAStartup(MAKEWORD(2, 0), &m_wsaData);
#endif

	this->serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (this->serverSocket < 0)
	{
		std::cout << "Failed to create a socket." << std::endl;
		exit(-1);
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

inline std::regex pathToRegex(std::string route);
inline std::vector<std::string> getParamNames(std::string route);
inline std::map<std::string, std::string> getParamsData(Route route, std::string path);
void HttpServer::_listen()
{
#if defined(__linux__)
	int opt = 1;
	if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
	{
		std::cout << "setsockopt SO_REUSEADDR." << std::endl;
		exit(-1);
	};
#endif

	if (bind(this->serverSocket, (struct sockaddr*)&socketAddress, sizeof(socketAddress)) < 0)
	{
		std::cout << "Failed to bind address." << std::endl;
#	if defined(_WIN32)
		closesocket(this->serverSocket);
#	elif defined(__linux)
		::close(this->serverSocket);
#	endif
		exit(-1);
	};

	if (::listen(this->serverSocket, this->mMaxConnections) < 0)
	{
		std::cout << "Failed to listen." << std::endl;
		exit(-1);
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
			bool match = std::regex_match(req.route(), route.route);
			if (true == match)
			{
				std::map<std::string, std::string> params = getParamsData(route, req.route());
				req.mParams = params;
				route.handle(req, res);

				/*bool nextRoute = false;
				std::function<void(void)> next = [&nextRoute] { nextRoute = true; };
				route.handle(req, res, next);

				if (!nextRoute)
					break;*/
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

void HttpServer::_use(std::string method, std::string route, RequestHandler handler)
{
	std::regex routeRegex = pathToRegex(route);
	std::vector<std::string> params = getParamNames(route);
	if (this->routes.find(method) == this->routes.end())
	{
		this->routes[method].push_back({ routeRegex, params, handler });
		return;
	};

	this->routes[method].push_back({ routeRegex, params, handler });
};

inline std::regex pathToRegex(std::string route)
{
	std::regex regex(R"([\/,\-@])");
	route = std::regex_replace(route, std::regex(R"(\/:([^\/]+)\?)"), "(?:\\/([^\\/]+))?");
	route = std::regex_replace(route, std::regex(R"(\/:([^\/\?(]+))"), "\\/([^\\/]+)");
	route = std::regex_replace(route, std::regex(R"(\*)"), "(.*)");
	route = std::regex_replace(route, std::regex(R"(\/$)"), "");

	route.append("(?:\\/)?");
	return std::regex(std::string("^").append(route).append("$"));
};

inline std::vector<std::string> getParamNames(std::string route)
{
	std::vector<std::string> paramNames;
	std::regex paramRegex(R"(:([^\/\?]+)\??)");
	std::sregex_iterator iter(route.begin(), route.end(), paramRegex);
	std::sregex_iterator end;
	while (iter != end) {
		paramNames.push_back((*iter)[1]);
		++iter;
	};

	return paramNames;
};

inline std::map<std::string, std::string> getParamsData(Route route, std::string path)
{
	std::smatch match;
	std::regex_match(path, match, route.route);
	std::map<std::string, std::string> params;
	for (size_t i = 0; i < route.params.size(); ++i)
		params[route.params[i]] = match[i + 1].str();

	return params;
};