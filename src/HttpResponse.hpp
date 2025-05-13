#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#if defined(_WIN32)
    #include <WinSock2.h>
#elif defined(__linux__)
    #include <unistd.h>
#endif

#include "HttpRequest.hpp"
#include "HttpStatus.hpp"

#if defined(_WIN32)
    typedef SOCKET Socket_t;
#elif defined(__unix__)
    typedef int Socket_t;
#endif

class HttpResponse
{
private:
    bool m_HeadersSent = false;
    HttpStatus::Code m_StatusCode = HttpStatus::OK;
    std::unordered_map<std::string, std::string> m_Headers;

protected:
    Socket_t m_ClientSocket;
    HttpRequest m_Request;

public:
    HttpResponse(Socket_t clientSocket, const HttpRequest& req) :
        m_ClientSocket(clientSocket),
        m_Request(req) {};

    bool send(std::string data);

    void setHeader(const std::string& key, const std::string& value) { this->m_Headers[key] = value; };
    std::string getHeader(const std::string& key) { return this->m_Headers[key]; };
    bool removeHeader(const std::string& key) { return this->m_Headers.erase(key) != 0; };

private:
    std::ostringstream toHttpString();
    bool sendToSocket(const std::string& data);
    void closeSocket() const;
};

#endif // !HTTPRESPONSE_HPP
