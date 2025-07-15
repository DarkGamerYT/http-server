#include "HttpServer.hpp"

unsigned int HttpServer::s_MaxWorkerThreads = std::max(1u, std::thread::hardware_concurrency());
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
    try {
        this->close();
    } catch (...) {};
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
    if (!this->m_bIsRunning)
        return;
    
    this->m_bIsRunning = false;
    
    // Notify all waiting threads to wake up and exit
    m_QueueCondVar.notify_all();

    // Join all worker threads
    for (auto& thread : m_WorkerThreads) {
        if (!thread.joinable())
            continue;
        
        thread.join();
    };
    
    if (this->m_ServerSocket == -1)
        return;

#if defined(_WIN32)
    closesocket(this->m_ServerSocket);
    WSACleanup();
#elif defined(__unix__) || defined(__APPLE__)
    ::close(this->m_ServerSocket);
#endif
        
    this->m_ServerSocket = -1;
};

void HttpServer::listen()
{
#if defined(__unix__) || defined(__APPLE__)
    int opt = 1;
    if (setsockopt(this->m_ServerSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
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
    
    m_WorkerThreads.resize(s_MaxWorkerThreads);
    for (auto i = 0; i < s_MaxWorkerThreads; i++) {
        m_WorkerThreads[i] = std::thread(&HttpServer::proccessRequests, this, i);
    };

    this->receiveConnections();
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
            m_RequestQueue.emplace(clientSocket, clientAddress);
        };
        m_QueueCondVar.notify_one(); // Wake up one waiting worker
    };
};

void HttpServer::proccessRequests(int workerId)
{
    while (this->m_bIsRunning)
    {
        sockaddr_in clientAddress{};
        Socket_t clientSocket;
        {
            std::unique_lock lock(m_QueueMutex);
            m_QueueCondVar.wait(lock, [this] {
                return !m_RequestQueue.empty() || !m_bIsRunning;
            });
            
            if (!m_bIsRunning && m_RequestQueue.empty())
                break;

            const auto& [ socket, address ] = m_RequestQueue.front();
            clientSocket = socket;
            clientAddress = address;
            m_RequestQueue.pop();
        };

        std::vector<char> buffer(s_MaxBufferSize);
    #if defined(_WIN32)
        int bytesReceived = recv(clientSocket, buffer.data(), static_cast<int>(buffer.size()), 0);
    #elif defined(__unix__) || defined(__APPLE__)
        ssize_t bytesReceived = read(clientSocket, buffer.data(), buffer.size());
    #endif

        if (bytesReceived < 1)
            return;

        std::string data(buffer.data(), bytesReceived);
        HttpRequest request{ clientSocket, data };

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

inline std::string extractPrefix(const std::string& regexPath)
{
    size_t i = 0;
    while (i < regexPath.size()) {
        char character = regexPath[i];
        if (
            std::isalnum(character)
            || character == '^'
            || character == '/'
            || character == '_'
            || character == '-'
        ) {
            ++i;
            continue;
        };

        break;
    };

    return regexPath.substr(0, i);
};

RequestHandler_t HttpServer::useStatic(const std::string& directory)
{
    return [directory](const HttpRequest& request, HttpResponse response) {
        const std::string& fullPath = request.getPath();
        const std::string& originalPattern = request.getOriginalPath();

        std::string routePrefix = extractPrefix(originalPattern);
        if (fullPath.find(routePrefix) != 0) {
            response.setStatus(HttpStatus::NotFound);
            response.setHeader("Content-Type", "text/plain");
            response.send("Not Found");
            return;
        }

        std::string relative = fullPath.substr(routePrefix.length());
        if (!relative.empty() && relative.front() == '/')
            relative.erase(0, 1);

        std::filesystem::path requestedPath = std::filesystem::path(directory) / relative;
        std::error_code error;

        auto canonicalPath = std::filesystem::weakly_canonical(requestedPath, error);
        auto canonicalRoot = std::filesystem::weakly_canonical(directory, error);

        if (
            error
            || canonicalPath.string().find(canonicalRoot.string()) != 0
            || canonicalPath.filename().string().starts_with("."))
        {
            response.setStatus(HttpStatus::Forbidden);
            response.setHeader("Content-Type", "text/plain");
            response.send("Forbidden");
            return;
        };

        if (
            !std::filesystem::exists(canonicalPath) ||
            !std::filesystem::is_regular_file(canonicalPath))
        {
            response.setStatus(HttpStatus::NotFound);
            response.setHeader("Content-Type", "text/plain");
            response.send("File Not Found");
            return;
        };

        response.sendFile(canonicalPath);
    };
};
