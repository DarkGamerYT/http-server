#include <print>

#include <openssl/sha.h>
#include "util/Base64.hpp"

#include "HttpServer.hpp"

unsigned int HttpServer::sMaxWorkerThreads = std::max(1u, std::thread::hardware_concurrency());
HttpServer::HttpServer(const bool enableWebSockets, const HttpVersion::Version version)
{
    this->b_mEnableWebSockets = enableWebSockets;
    this->mVersion = version;

    if (this->mVersion != HttpVersion::HTTP_1_1)
        throw std::runtime_error("Unsupported HTTP version");

#if defined(_WIN32)
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        throw std::runtime_error("WSAStartup failed");
    };
#endif

    this->mServerSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (this->mServerSocket < 0) {
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
    this->mSocketAddress.sin_port = htons(port);
    this->mSocketAddress.sin_family = AF_INET;
    this->mSocketAddress.sin_addr.s_addr = INADDR_ANY;

    this->listen();
};

void HttpServer::listen(const char* address, unsigned short port)
{
    this->mSocketAddress.sin_port = htons(port);
    this->mSocketAddress.sin_family = AF_INET;
    inet_pton(AF_INET, address, &(this->mSocketAddress.sin_addr.s_addr));

    this->listen();
};

void HttpServer::close()
{
    if (!this->b_mIsRunning)
        return;
    
    this->b_mIsRunning = false;
    
    // Notify all waiting threads to wake up and exit
    mQueueCondVar.notify_all();

    // Join all worker threads
    for (auto& thread : mWorkerThreads) {
        if (!thread.joinable())
            continue;
        
        thread.join();
    };
    
    if (this->mServerSocket == -1)
        return;

#if defined(_WIN32)
    closesocket(this->mServerSocket);
    WSACleanup();
#elif defined(__unix__) || defined(__APPLE__)
    ::close(this->mServerSocket);
#endif
        
    this->mServerSocket = -1;
};

void HttpServer::listen()
{
    constexpr int opt = 1;
    // SO_REUSEADDR
#if defined(_WIN32)
    if (setsockopt(this->mServerSocket, SOL_SOCKET, SO_REUSEADDR,
                   reinterpret_cast<const char*>(&opt), sizeof(opt)) < 0)
#elif defined(__unix__) || defined(__APPLE__)
    if (setsockopt(this->mServerSocket, SOL_SOCKET, SO_REUSEADDR,
                   &opt, sizeof(opt)) < 0)
#endif
    {
        throw std::runtime_error("Failed to set SO_REUSEADDR");
    };

    // TCP_NODELAY
#if defined(_WIN32)
    if (setsockopt(this->mServerSocket, IPPROTO_TCP, TCP_NODELAY,
                   reinterpret_cast<const char*>(&opt), sizeof(opt)) < 0)
#elif defined(__unix__) || defined(__APPLE__)
    if (setsockopt(this->mServerSocket, IPPROTO_TCP, TCP_NODELAY,
                   &opt, sizeof(opt)) < 0)
#endif
    {
        throw std::runtime_error("Failed to set TCP_NODELAY");
    };

    if (bind(this->mServerSocket, reinterpret_cast<sockaddr *>(&mSocketAddress), sizeof(mSocketAddress)) < 0)
    {
        throw std::runtime_error("Failed to bind to socket");
    };

    if (::listen(this->mServerSocket, sMaxConnections) < 0)
    {
        throw std::runtime_error("Failed to listen");
    };

    this->b_mIsRunning = true;
    
    mWorkerThreads.resize(sMaxWorkerThreads);
    for (auto i = 0; i < sMaxWorkerThreads; i++) {
        mWorkerThreads[i] = std::thread(&HttpServer::processRequests, this, i);
    };

    this->receiveConnections();
};

void HttpServer::receiveConnections()
{
    while (this->b_mIsRunning)
    {
        sockaddr_in clientAddress{};
        socklen_t addressLength = sizeof(clientAddress);

        Socket_t clientSocket = accept(this->mServerSocket, reinterpret_cast<struct sockaddr *>(&clientAddress), &addressLength);
        if (clientSocket < 0)
            continue;

        {
            std::unique_lock lock(mQueueMutex);
            mRequestQueue.emplace(clientSocket, clientAddress);
        };
        mQueueCondVar.notify_one(); // Wake up one waiting worker
    };
};

template <typename Map, typename Handler>
 std::optional<std::pair<std::string, Handler>> findRoute(const std::string& path, Map routes) {
    for (const auto& route : routes | std::views::keys)
    {
        bool matches = false;
        try {
            const std::regex pattern{ route };
            matches = std::regex_match(path, pattern);
        }
        catch (...) {
            matches = (route == path);
        };

        if (true == matches)
            return std::make_pair(path, routes.at(route));
    };

    return std::nullopt;
};

void HttpServer::processRequests(int workerId)
{
    while (this->b_mIsRunning)
    {
        //sockaddr_in clientAddress{};
        Socket_t clientSocket;
        {
            std::unique_lock lock(mQueueMutex);
            mQueueCondVar.wait(lock, [this] {
                return !mRequestQueue.empty() || !b_mIsRunning;
            });
            
            if (!b_mIsRunning && mRequestQueue.empty())
                break;

            const auto& [ socket, address ] = mRequestQueue.front();
            clientSocket = socket;
            //clientAddress = address;
            mRequestQueue.pop();
        };

        std::vector<uint8_t> buffer(sMaxBufferSize);
    #if defined(_WIN32)
        const int bytesReceived = recv(clientSocket, reinterpret_cast<char *>(buffer.data()), static_cast<int>(buffer.size()), 0);
    #elif defined(__unix__) || defined(__APPLE__)
        const ssize_t bytesReceived = read(clientSocket, buffer.data(), buffer.size());
    #endif

        if (bytesReceived < 1)
            return;

        std::string data(reinterpret_cast<const char*>(buffer.data()), bytesReceived);
        HttpRequest request{ clientSocket, data };

        const std::string& path = request.getPath();
        const HttpMethod::Method& method = request.getMethod();

        if (HttpServer::isUpgradeRequest(request))
        {
            if (this->b_mEnableWebSockets)
                this->upgradeConnection(clientSocket, request, buffer);

        #if defined(_WIN32)
            ::closesocket(clientSocket);
        #elif defined(__unix__) || defined(__APPLE__)
            ::close(clientSocket);
        #endif
            return;
        };

        const auto& route =
            findRoute<decltype(this->mRoutes), RouteHandlers>(path, this->mRoutes);

        if (!route.has_value())
            continue;

        const auto& handlers = route.value().second;
        if (!handlers.contains(method))
            continue;

        HttpResponse response{ clientSocket, request, this->mVersion };
        response.setHeader("Content-Type", "text/plain");

        const std::vector<Middleware>& chain = handlers.at(method);
        {
            size_t i = 0;
            std::function<void()> next = [&]() {
                if (i >= chain.size())
                    return;

                auto& mw = chain[i++];
                mw(request, response, next);
            };

            next();
        };
    };
};


inline std::string extractPrefix(const std::string& regexPath)
{
    size_t i = 0;
    while (i < regexPath.size()) {
        if (const char character = regexPath[i];
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

Middleware HttpServer::useStatic(const std::string& directory)
{
    return [directory](const HttpRequest& request, HttpResponse& response, const NextFn&) {
        const std::string& fullPath = request.getPath();
        const std::string& originalPattern = request.getOriginalPath();

        const std::string& routePrefix = extractPrefix(originalPattern);
        if (fullPath.find(routePrefix) != 0) {
            response.setStatus(HttpStatus::NotFound);
            response.setHeader("Content-Type", "text/plain");
            response.send("Not Found");
            return;
        };

        std::string relative = fullPath.substr(routePrefix.length());
        if (!relative.empty() && relative.front() == '/')
            relative.erase(0, 1);

        const std::filesystem::path& requestedPath = std::filesystem::path(directory) / relative;

        std::error_code error;
        const auto canonicalPath = std::filesystem::weakly_canonical(requestedPath, error);
        const auto canonicalRoot = std::filesystem::weakly_canonical(directory, error);

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
    const std::string& path = request.getPath();
    const auto& route =
        findRoute<decltype(this->mSockets), WebSocketHandler>(path, this->mSockets);

    if (!route.has_value())
        return;

    const auto& key = request.getHeader("Sec-WebSocket-Key");
    if (request.getMethod() != HttpMethod::GET || !key.has_value())
    {
#if defined(_WIN32)
        ::closesocket(socket);
#elif defined(__unix__) || defined(__APPLE__)
        ::close(socket);
#endif
        return;
    };

    HttpResponse response{ socket, request, this->mVersion, false };
    HttpServer::upgradeWebSocket(response, key.value());

    const auto& handlers = route.value().second;
    std::function next = [&]() {
        response.send();

        WebSocket webSocket{ socket };
        handlers.onOpen(webSocket);

        std::vector<uint8_t> fragmentBuffer;
        uint8_t fragmentOpcode = 0;
        bool isFragmented = false;
        while (true)
        {
    #if defined(_WIN32)
            const int received = recv(socket, reinterpret_cast<char *>(buffer.data()), static_cast<int>(buffer.size()), 0);
    #elif defined(__unix__) || defined(__APPLE__)
            const ssize_t received = read(socket, buffer.data(), buffer.size());
    #endif

            if (received <= 0)
                break;

            const uint8_t opcode = buffer[0] & 0x0F;
            if (opcode == 0x8) {
                handlers.onClose(webSocket);
                HttpServer::sendToSocket(socket, std::string{ static_cast<char>(0x88), 0x00 });
                break;
            };

            const bool isFinal = buffer[0] & 0x80;
            const uint8_t lengthCode = buffer[1] & 0x7F;

            size_t payloadSize = lengthCode;
            size_t offset = 2;
            if (lengthCode == 126) {
                if (received < 4)
                    continue;

                payloadSize = (static_cast<size_t>(buffer[2]) << 8) | buffer[3];
                offset += 2;
            }
            else if (lengthCode == 127) {
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
            std::vector<uint8_t> payload;
            payload.reserve(payloadSize);

            for (size_t i = 0; i < payloadSize; ++i)
                payload.push_back(static_cast<char>(buffer[offset + i] ^ maskingKey[i % 4]));

            switch (opcode)
            {
                case 0x1: {
                    // Text frame - possibly fragmented
                    if (!isFinal)
                    {
                        fragmentBuffer = payload;
                        fragmentOpcode = opcode;
                        isFragmented = true;
                        continue;
                    };

                    const std::string text{payload.begin(), payload.end()};
                    handlers.onText(webSocket, text);
                    break;
                };

                case 0x2: {
                    // Binary frame - possibly fragmented
                    if (!isFinal)
                    {
                        fragmentBuffer = payload;
                        fragmentOpcode = opcode;
                        isFragmented = true;
                        continue;
                    };

                    handlers.onBinary(webSocket, payload);
                    break;
                };

                case 0x0: {
                    // Continuation frame
                    if (!isFragmented)
                        continue; // Unexpected continuation

                    fragmentBuffer.insert(fragmentBuffer.end(), payload.begin(), payload.end());
                    if (!isFinal)
                        continue;

                    if (fragmentOpcode == 0x1) {
                        const std::string text{fragmentBuffer.begin(), fragmentBuffer.end()};
                        handlers.onText(webSocket, text);
                    }
                    else if (fragmentOpcode == 0x2) {
                        handlers.onBinary(webSocket, fragmentBuffer);
                    };

                    fragmentBuffer.clear();
                    isFragmented = false;
                    break;
                };

                default:
                    std::println("Unsupported opcode: {}", opcode);
                    break;
            };
        };
    };

    std::visit([&]<typename T0>(T0&& fn) {
        using FnType = std::decay_t<T0>;
        if constexpr (std::is_same_v<FnType, Middleware>) {
            fn(request, response, next);
        }
        else {
            fn(request, response);
            next();
        };
    }, handlers.onRequest);
};

void HttpServer::upgradeWebSocket(HttpResponse& response, const std::string& mainKey) {
    response.setStatus(HttpStatus::SwitchingProtocols);
    response.setHeader("Connection", "Upgrade");
    response.setHeader("Upgrade", "websocket");

    {
        std::string key{mainKey};
        const auto& input = key.append("258EAFA5-E914-47DA-95CA-C5AB0DC85B11");

        unsigned char hash[SHA_DIGEST_LENGTH];
        SHA1(reinterpret_cast<const unsigned char *>(input.c_str()), input.size(), hash);

        const std::string binary(reinterpret_cast<char*>(hash), SHA_DIGEST_LENGTH);
        const auto& output = Base64::encode(binary);

        response.setHeader("Sec-WebSocket-Accept", output);
    };
};

bool HttpServer::isUpgradeRequest(const HttpRequest& request) {
    if (const auto& connection = request.getHeader("Connection");
        !connection.has_value()
        || (connection.value() != "Upgrade" && connection.value() != "upgrade"))
        return false;

    if (const auto& upgrade = request.getHeader("Upgrade");
        !upgrade.has_value()
        || upgrade.value() != "websocket")
        return false;

    if (const auto& key = request.getHeader("Sec-WebSocket-key");
        !key.has_value())
        return false;

    return true;
};

void HttpServer::sendToSocket(const Socket_t socket, const std::string& data) {
    const char* buffer = data.c_str();
    const size_t bytesToSend = data.size();

    size_t totalBytesSent = 0;
    while (totalBytesSent < data.size())
    {
#if defined(_WIN32)
        const int bytesSent = ::send(socket, buffer + totalBytesSent, static_cast<int>(bytesToSend - totalBytesSent), 0);
#elif defined(__unix__) || defined(__APPLE__)
        const ssize_t bytesSent = ::write(socket, buffer + totalBytesSent, bytesToSend - totalBytesSent);
#endif
        if (bytesSent < 0)
            break;

        totalBytesSent += bytesSent;
    };
};