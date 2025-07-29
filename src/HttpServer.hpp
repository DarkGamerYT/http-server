#ifndef HTTPSERVER_HPP
#define HTTPSERVER_HPP

#include <openssl/sha.h>

#ifndef NOMINMAX
    #define NOMINMAX
#endif /* NOMINMAX */

#if defined(_WIN32)
    #include <WinSock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "Ws2_32.lib")
#elif defined(__unix__) || defined(__APPLE__)
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <netinet/tcp.h>
    #include <arpa/inet.h>
    #include <unistd.h>
#endif

#include <stdexcept>
#include <cerrno>
#include <iostream>
#include <thread>
#include <vector>
#include <functional>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <regex>
#include <variant>
#include <array>
#include <ranges>
#include <filesystem>
#include <map>

#include "util/HttpMethod.hpp"
#include "util/Base64.hpp"

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "WebSocket.hpp"

typedef std::function<void(HttpRequest, HttpResponse)> RequestHandler_t;
typedef std::unordered_map<HttpMethod::Method, RequestHandler_t> RouteHandlers_t;

class HttpServer
{
private:
    static constexpr int sMaxConnections = 1024;
    static constexpr int sMaxBufferSize = 65536;
    static unsigned int sMaxWorkerThreads;

    bool b_mEnableWebSockets = false;
    bool b_mIsRunning = false;
    std::vector<std::thread> mWorkerThreads;
    std::queue<std::pair<Socket_t, sockaddr_in>> mRequestQueue;
    std::mutex mQueueMutex;
    std::condition_variable mQueueCondVar;

protected:
    Socket_t mServerSocket{ 0 };
    sockaddr_in mSocketAddress{};

    std::map<std::string, RouteHandlers_t> mRoutes;
    std::map<std::string, WebSocketHandler> mSockets;

public:
	explicit HttpServer(bool enableWebSockets = false);
	~HttpServer();

    void use(const std::string& route, HttpMethod::Method method,
        const RequestHandler_t& callback);

    void websocket(const std::string& route, const WebSocketHandler& handler);

    void listen(unsigned short port);
    void listen(const char* address, unsigned short port);
    void close();

    static RequestHandler_t useStatic(const std::string& directory);
    static void sendToSocket(Socket_t socket, const std::string& data);

private:
    void listen();
    void receiveConnections();
    void proccessRequests(int workerId);

    static bool isUpgradeRequest(const HttpRequest& request);
    void upgradeConnection(Socket_t socket, const HttpRequest& request, std::vector<uint8_t>& buffer);
    static void upgradeWebSocket(Socket_t socket, const HttpRequest& request, std::string& key);
};

#endif // !HTTPSERVER_HPP
