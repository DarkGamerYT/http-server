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

#if defined(_WIN32)
typedef SOCKET Socket_t;
#elif defined(__unix__)
typedef int Socket_t;
#endif

#include "HttpMethod.hpp"

typedef std::function<void()> RequestHandler_t;

class HttpServer
{
private:
    static constexpr int s_MaxConnections = 1024;
    static constexpr int s_MaxBufferSize = 65536;
    static constexpr int s_MaxWorkerThreads = 4;

    bool m_bIsRunning = false;
    std::thread m_WorkerThreads[s_MaxWorkerThreads];
    std::queue<int> m_RequestQueue;
    std::mutex m_QueueMutex;
    std::condition_variable m_QueueCondVar;

    Socket_t m_ServerSocket{ 0 };
    sockaddr_in m_SocketAddress{};
    std::thread m_ListenerThread;

    std::unordered_map<std::string,
        std::unordered_map<HttpMethod::Method, RequestHandler_t>> m_Routes;

public:
	HttpServer();
	~HttpServer();

    void use(const std::string& route, HttpMethod::Method method,
        const RequestHandler_t callback);

    void listen(short port);
    void listen(const char* address, short port);
    void close();

private:
    void listen();
    void receiveConnections();
    void proccessRequests(int workerId);
};

#endif // !HTTPSERVER_HPP
