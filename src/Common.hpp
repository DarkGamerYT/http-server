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

#endif //HTTP_SERVER_COMMON_HPP