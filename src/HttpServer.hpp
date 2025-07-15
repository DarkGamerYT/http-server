#ifndef HTTPSERVER_HPP
#define HTTPSERVER_HPP

#if defined(_WIN32)
    #include <WinSock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "Ws2_32.lib")
#elif defined(__unix__)
    #include <sys/socket.h>
    #include <netinet/in.h>
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
#include <filesystem>
#include <map>

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "util/HttpMethod.hpp"

typedef std::function<void(HttpRequest, HttpResponse)> RequestHandler_t;
typedef std::unordered_map<HttpMethod::Method, RequestHandler_t> RouteHandlers_t;

class HttpServer
{
private:
    static constexpr int s_MaxConnections = 1024;
    static constexpr int s_MaxBufferSize = 65536;
    static unsigned int s_MaxWorkerThreads;

    bool m_bIsRunning = false;
    std::vector<std::thread> m_WorkerThreads;
    std::queue<std::pair<Socket_t, sockaddr_in>> m_RequestQueue;
    std::mutex m_QueueMutex;
    std::condition_variable m_QueueCondVar;

    Socket_t m_ServerSocket{ 0 };
    sockaddr_in m_SocketAddress{};

    std::map<std::string, RouteHandlers_t> m_Routes;

public:
	HttpServer();
	~HttpServer();

    void use(const std::string& route, HttpMethod::Method method,
        const RequestHandler_t& callback);

    void listen(unsigned short port);
    void listen(const char* address, unsigned short port);
    void close();

    static RequestHandler_t useStatic(const std::string& directory);

private:
    void listen();
    void receiveConnections();
    void proccessRequests(int workerId);
};

#endif // !HTTPSERVER_HPP
