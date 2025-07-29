#include "HttpServer.hpp"

unsigned int HttpServer::s_MaxWorkerThreads = std::max(1u, std::thread::hardware_concurrency());
HttpServer::HttpServer(bool enableWebSockets)
{
    this->m_bEnableWebSockets = enableWebSockets;

#if defined(_WIN32)
    WSADATA m_wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &m_wsaData) != 0) {
        throw std::runtime_error("WSAStartup failed");
    };
#endif

    this->m_ServerSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (this->m_ServerSocket < 0) {
        throw std::runtime_error("Failed to create socket");
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
    this->m_SocketAddress.sin_family = AF_INET;
    this->m_SocketAddress.sin_addr.s_addr = INADDR_ANY;

    this->listen();
};

void HttpServer::listen(const char* address, unsigned short port)
{
    this->m_SocketAddress.sin_port = htons(port);
    this->m_SocketAddress.sin_family = AF_INET;
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
    int opt = 1;
    // SO_REUSEADDR
#if defined(_WIN32)
    if (setsockopt(this->m_ServerSocket, SOL_SOCKET, SO_REUSEADDR,
                   reinterpret_cast<const char*>(&opt), sizeof(opt)) < 0)
#elif defined(__unix__) || defined(__APPLE__)
    if (setsockopt(this->m_ServerSocket, SOL_SOCKET, SO_REUSEADDR,
                   &opt, sizeof(opt)) < 0)
#endif
    {
        throw std::runtime_error("Failed to set SO_REUSEADDR");
    };

    // TCP_NODELAY
#if defined(_WIN32)
    if (setsockopt(this->m_ServerSocket, IPPROTO_TCP, TCP_NODELAY,
                   reinterpret_cast<const char*>(&opt), sizeof(opt)) < 0)
#elif defined(__unix__) || defined(__APPLE__)
    if (setsockopt(this->m_ServerSocket, IPPROTO_TCP, TCP_NODELAY,
                   &opt, sizeof(opt)) < 0)
#endif
    {
        throw std::runtime_error("Failed to set TCP_NODELAY");
    };

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

        std::vector<uint8_t> buffer(s_MaxBufferSize);
    #if defined(_WIN32)
        int bytesReceived = recv(clientSocket, reinterpret_cast<char *>(buffer.data()), static_cast<int>(buffer.size()), 0);
    #elif defined(__unix__) || defined(__APPLE__)
        ssize_t bytesReceived = read(clientSocket, buffer.data(), buffer.size());
    #endif

        if (bytesReceived < 1)
            return;

        std::string data(reinterpret_cast<const char*>(buffer.data()), bytesReceived);
        HttpRequest request{ clientSocket, data };

        const std::string& path = request.getPath();
        const auto& method = request.getMethod();

        if (HttpServer::isUpgradeRequest(request))
        {
            if (this->m_bEnableWebSockets)
            {
                HttpServer::upgradeConnection(clientSocket, request, buffer);
            }
            else
            {
            #if defined(_WIN32)
                ::closesocket(clientSocket);
            #elif defined(__unix__) || defined(__APPLE__)
                ::close(clientSocket);
            #endif
            };
            return;
        };

        RouteHandlers_t handlers{};
        bool bExists = false;
        for (const auto& [route, data] : m_Routes)
        {
            bool matches = false;
            try {
                std::regex pattern{ route };
                matches = std::regex_match(path, pattern);
            }
            catch (...) {
                matches = (route == path);
            };

            if (false == matches)
                continue;

            bExists = true;
            handlers = data;
            request.setOriginalPath(route);
            break;
        };

        if (false == bExists || !handlers.contains(method))
            continue;

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

void HttpServer::websocket(const std::string &route, const WebSocketHandler& handler) {
    this->m_Sockets[route] = handler;
};


inline std::string extractPrefix(const std::string& regexPath)
{
    size_t i = 0;
    while (i < regexPath.size()) {
        const char character = regexPath[i];
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

void HttpServer::upgradeConnection(Socket_t socket, const HttpRequest& request, std::vector<uint8_t>& buffer) {
    const auto& path = request.getPath();
    if (request.getMethod() != HttpMethod::GET
        || !this->m_Sockets.contains(path))
    {
#if defined(_WIN32)
        ::closesocket(socket);
#elif defined(__unix__) || defined(__APPLE__)
        ::close(socket);
#endif
        return;
    };

    auto key = request.getHeaders().at("Sec-WebSocket-Key");
    HttpServer::upgradeWebSocket(socket, request, key);

    WebSocket webSocket{ socket };
    const auto&[onOpen, onMessage, onClose] = this->m_Sockets.at(path);
    onOpen(webSocket);

    std::string fragmentBuffer;
    bool isFragmented = false;
    while (true)
    {
#if defined(_WIN32)
        int received = recv(socket, reinterpret_cast<char *>(buffer.data()), static_cast<int>(buffer.size()), 0);
#elif defined(__unix__) || defined(__APPLE__)
        ssize_t received = read(socket, buffer.data(), buffer.size());
#endif

        if (received <= 0)
            break;

        const uint8_t opcode = buffer[0] & 0x0F;
        if (opcode == 0x8) {
            onClose(webSocket);
            HttpServer::sendToSocket(socket, std::string{ static_cast<char>(0x88), 0x00 });
            break;
        };

        bool isFinal = buffer[0] & 0x80;
        uint8_t lengthCode = buffer[1] & 0x7F;

        size_t payloadSize = lengthCode;
        size_t offset = 2;
        if (lengthCode == 126) {
            if (received < 4)
                continue;

            payloadSize = (static_cast<size_t>(buffer[2]) << 8) | buffer[3];
            offset += 2;
        } else if (lengthCode == 127) {
            if (received < 10)
                continue;

            payloadSize = 0;
            for (int i = 0; i < 8; ++i)
                payloadSize = (payloadSize << 8) | static_cast<size_t>(buffer[2 + i]);

            offset += 8;
        };

        if (buffer.size() < offset + 4 + payloadSize)
            continue; // Wait for more data

        std::array<uint8_t, 4> maskingKey{};
        std::copy_n(buffer.data() + offset, 4, maskingKey.begin());
        offset += 4;

        // Decode payload
        std::string message;
        message.reserve(payloadSize);

        for (size_t i = 0; i < payloadSize; ++i)
            message += static_cast<char>(buffer[offset + i] ^ maskingKey[i % 4]);

        switch (opcode)
        {
            case 0x1: {
                // Text frame - possibly fragmented
                if (!isFinal)
                {
                    fragmentBuffer = message;
                    isFragmented = true;
                    continue;
                };

                onMessage(webSocket, message);
                break;
            };

            case 0x0: {
                // Continuation frame
                if (!isFragmented)
                    continue; // Unexpected continuation

                fragmentBuffer += message;

                if (isFinal)
                {
                    onMessage(webSocket, fragmentBuffer);
                    fragmentBuffer.clear();
                    isFragmented = false;
                };

                break;
            };

            default:
                // Unsupported opcode (e.g., binary or ping)
                break;
        };
    };
};

void HttpServer::upgradeWebSocket(Socket_t socket, const HttpRequest& request, std::string& key) {
    HttpResponse response{ socket, request, false };

    response.setStatus(HttpStatus::SwitchingProtocols);
    response.setHeader("Connection", "Upgrade");
    response.setHeader("Upgrade", "websocket");

    {
        const auto& input = key.append("258EAFA5-E914-47DA-95CA-C5AB0DC85B11");

        unsigned char hash[SHA_DIGEST_LENGTH];
        SHA1(reinterpret_cast<const unsigned char *>(input.c_str()), input.size(), hash);

        std::string binary(reinterpret_cast<char*>(hash), SHA_DIGEST_LENGTH);
        const auto& output = Base64::encode(binary);

        response.setHeader("Sec-WebSocket-Accept", output);
    };

    response.send();
};

bool HttpServer::isUpgradeRequest(const HttpRequest& request) {
    const auto& headers = request.getHeaders();
    if (!headers.contains("Connection")
        || headers.at("Connection") != "Upgrade")
        return false;

    if (!headers.contains("Upgrade")
        || headers.at("Upgrade") != "websocket")
        return false;

    if (!headers.contains("Sec-WebSocket-Key"))
        return false;

    return true;
};

void HttpServer::sendToSocket(Socket_t socket, const std::string& data) {
    const char* buffer = data.c_str();
    size_t bytesToSend = data.size();
    size_t totalBytesSent = 0;
    while (totalBytesSent < data.size())
    {
#if defined(_WIN32)
        int bytesSent = ::send(socket, buffer + totalBytesSent, static_cast<int>(bytesToSend - totalBytesSent), 0);
#elif defined(__unix__) || defined(__APPLE__)
        ssize_t bytesSent = ::write(socket, buffer + totalBytesSent, bytesToSend - totalBytesSent);
#endif
        if (bytesSent < 0)
            break;

        totalBytesSent += bytesSent;
    };
};