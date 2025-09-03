#ifndef HTTPSERVER_HPP
#define HTTPSERVER_HPP

#include <thread>
#include <vector>
#include <functional>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <regex>
#include <variant>
#include <ranges>
#include <unordered_map>

#include "util/HttpMethod.hpp"
#include "util/HttpVersion.hpp"

#include "Common.hpp"

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "WebSocket.hpp"

using RouteHandlers = std::unordered_map<HttpMethod::Method, std::vector<Middleware>>;
class HttpServer
{
private:
    static constexpr int sMaxConnections = 1024;
    static constexpr int sMaxBufferSize = 65536;
    static unsigned int sMaxWorkerThreads;

    bool b_mEnableWebSockets{ false };
    bool b_mIsRunning{ false };
    std::vector<std::thread> mWorkerThreads{};
    std::queue<std::pair<Socket_t, sockaddr_in>> mRequestQueue{};
    std::mutex mQueueMutex{};
    std::condition_variable mQueueCondVar{};

protected:
    Socket_t mServerSocket{ 0 };
    sockaddr_in mSocketAddress{};
    HttpVersion::Version mVersion{ HttpVersion::HTTP_1_1 };

    std::unordered_map<std::string, RouteHandlers> mRoutes{};
    std::unordered_map<std::string, WebSocketHandler> mSockets{};

public:
	explicit HttpServer(bool enableWebSockets = false, HttpVersion::Version version = HttpVersion::HTTP_1_1);
	~HttpServer();

    template<typename... Middlewares>
        requires (is_middlware<Middlewares> && ...)
    void use(const std::string& route, Middlewares... mws) {
        for (int i = HttpMethod::GET; i <= static_cast<int>(HttpMethod::PATCH); ++i)
        {
            const auto& method = static_cast<HttpMethod::Method>(i);
            this->use(route, method, std::move(mws...));
        };
    };

    template<typename... Middlewares>
        requires (is_middlware<Middlewares> && ...)
    void use(const std::string& route, HttpMethod::Method method, Middlewares... mws) {
        std::vector<Middleware> middlewares;
        (middlewares.push_back(
            []<typename T>(T middleware) {
                if constexpr (std::is_convertible_v<T, Middleware>)
                    return middleware;

                else return [middleware](const HttpRequest& req, HttpResponse& res, const NextFn& next) {
                    middleware(req, res);
                    next();
                };
            }(mws)
        ), ...);

        this->mRoutes[route].insert(
            std::make_pair(method, std::move(middlewares))
        );
    };

    void websocket(const std::string& route, const WebSocketHandler& handler) {
        this->mSockets[route] = handler;
    };

    void listen(unsigned short port);
    void listen(const char* address, unsigned short port);
    void close();

    static Middleware useStatic(const std::string& directory);
    static void sendToSocket(Socket_t socket, const std::string& data);

private:
    void listen();
    void receiveConnections();
    void processRequests(int workerId);

    void upgradeConnection(Socket_t socket, const HttpRequest& request, std::vector<uint8_t>& buffer);
    static void upgradeWebSocket(HttpResponse& response, const std::string& key) ;
    static bool isUpgradeRequest(const HttpRequest& request);
};

#endif // !HTTPSERVER_HPP
