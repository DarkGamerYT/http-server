#include "HttpServer.hpp"
HttpServer::HttpServer()
{
#if defined(_WIN32)
    WSADATA m_wsaData;
    (void)WSAStartup(MAKEWORD(2, 0), &m_wsaData);
#endif

    this->m_ServerSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (this->m_ServerSocket < 0)
    {
        throw std::runtime_error("Failed to create a socket");
    };
};

HttpServer::~HttpServer()
{
    this->close();
};

void HttpServer::listen(unsigned short port)
{
    this->m_SocketAddress.sin_port = htons(port);
    this->m_SocketAddress.sin_addr.s_addr = INADDR_ANY;

    this->listen();
};

void HttpServer::listen(const char* address, unsigned short port)
{
    this->m_SocketAddress.sin_port = htons(port);
    inet_pton(AF_INET, address, &(this->m_SocketAddress.sin_addr.s_addr));

    this->listen();
};

void HttpServer::close()
{
    m_ListenerThread.join();

#if defined(_WIN32)
    closesocket(this->m_ServerSocket);
    WSACleanup();
#elif defined(__unix__)
    ::close(this->m_ServerSocket);
#endif
};

void HttpServer::listen()
{
#if defined(__unix__)
    int opt = 1;
    if (setsockopt(this->m_ServerSocket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < 0)
    {
        throw std::runtime_error("Failed to set socket options");
    };
#endif

    this->m_SocketAddress.sin_family = AF_INET;
    if (bind(this->m_ServerSocket, reinterpret_cast<sockaddr *>(&m_SocketAddress), sizeof(m_SocketAddress)) < 0)
    {
        throw std::runtime_error("Failed to bind to socket");
    };

    if (::listen(this->m_ServerSocket, s_MaxConnections) < 0)
    {
        throw std::runtime_error("Failed to listen");
    };

    this->m_bIsRunning = true;
    m_ListenerThread = std::thread(&HttpServer::receiveConnections, this);
    for (int i = 0; i < s_MaxWorkerThreads; i++) {
        m_WorkerThreads[i] = std::thread(&HttpServer::proccessRequests, this, i);
    };
};

void HttpServer::receiveConnections()
{
    while (this->m_bIsRunning)
    {
        sockaddr_in clientAddress{};
        socklen_t addressLength = sizeof(clientAddress);

        Socket_t clientSocket = accept(this->m_ServerSocket, reinterpret_cast<struct sockaddr *>(&clientAddress), &addressLength);
        if (clientSocket < 0)
            continue;

        {
            std::unique_lock lock(m_QueueMutex);
            m_RequestQueue.push(clientSocket);
        };
        m_QueueCondVar.notify_one(); // Wake up one waiting worker
    };
};

void HttpServer::proccessRequests(int workerId)
{
    while (this->m_bIsRunning)
    {
        Socket_t clientSocket;
        {
            std::unique_lock lock(m_QueueMutex);
            m_QueueCondVar.wait(lock, [this] { return !m_RequestQueue.empty(); });

            clientSocket = m_RequestQueue.front();
            m_RequestQueue.pop();
        };

        std::vector<char> buffer(s_MaxBufferSize);
    #if defined(_WIN32)
        int bytesReceived = recv(clientSocket, buffer.data(), static_cast<int>(buffer.size()), 0);
    #elif defined(__unix__)
        int bytesReceived = read(clientSocket, buffer.data(), buffer.size());
    #endif

        if (bytesReceived < 1)
            return;

        std::string data(buffer.data(), bytesReceived);
        HttpRequest request{ data };

        const std::string& path = request.getPath();
        const auto& method = request.getMethod();

        bool bExists = false;
        RouteHandlers_t handlers{};
        for (const auto& [route, data] : m_Routes)
        {
            try {
                std::regex pattern{ route };
                if (!std::regex_match(path, pattern))
                {
                    continue;
                };
            }
            catch (const std::regex_error&) {
                if (route != path)
                {
                    continue;
                };
            };

            bExists = true;
            handlers = data;
            request.setOriginalPath(route);
            break;
        };

        if (false == bExists)
        {
            continue;
        };

        if (!handlers.contains(method))
        {
            continue;
        };

        const RequestHandler_t& handler = handlers.at(method);
        handler(request, { clientSocket, request });
    };
};

void HttpServer::use(const std::string& route, HttpMethod::Method method,
    const RequestHandler_t& callback)
{
    this->m_Routes[route].insert(
        std::make_pair(method, callback)
    );
};

RequestHandler_t HttpServer::useStatic(const std::string& directory)
{
    return [](const HttpRequest& request, HttpResponse response) {
        response.setHeader("Content-Type", "text/plain");
        response.send("This is a test");
    };
};