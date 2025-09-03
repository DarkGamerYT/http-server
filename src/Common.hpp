#ifndef HTTP_SERVER_COMMON_HPP
#define HTTP_SERVER_COMMON_HPP

#if defined(_WIN32)
    #include <WinSock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "Ws2_32.lib")
#elif defined(__unix__) || defined(__APPLE__)
    #include <unistd.h>
    #include <netinet/in.h>
    #include <netinet/tcp.h>
    #include <arpa/inet.h>
#endif

#if defined(_WIN32)
    typedef SOCKET Socket_t;
#elif defined(__unix__) || defined(__APPLE__)
    typedef int Socket_t;
#endif

#include <functional>

#include "util/HttpMethod.hpp"
#include "util/HttpVersion.hpp"

#include "Common.hpp"

class HttpRequest;
class HttpResponse;

using NextFn = std::function<void()>;
using Middleware = std::function<void(const HttpRequest&, HttpResponse&, NextFn)>;

template<typename T>
concept is_middlware =
    std::is_convertible_v<T, Middleware> ||
    std::is_invocable_r_v<void, T, const HttpRequest&, HttpResponse&>;

#endif //HTTP_SERVER_COMMON_HPP