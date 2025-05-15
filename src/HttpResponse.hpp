#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#if defined(_WIN32)
    #include <WinSock2.h>
#elif defined(__linux__)
    #include <unistd.h>
#endif

#if defined(_WIN32)
    typedef SOCKET Socket_t;
#elif defined(__unix__)
    typedef int Socket_t;
#endif

#include "HttpRequest.hpp"
#include "HttpStatus.hpp"

class HttpResponse
{
private:
    bool m_HeadersSent = false;
    HttpStatus::Code m_StatusCode = HttpStatus::OK;
    HeadersMap_t m_Headers;

protected:
    Socket_t m_ClientSocket;
    HttpRequest m_Request;

public:
    HttpResponse(Socket_t clientSocket, const HttpRequest& req) :
        m_ClientSocket(clientSocket),
        m_Request(req) {};

    HttpResponse& setStatus(HttpStatus::Code status) { this->m_StatusCode = status; return *this; };

    bool send(std::string data);
    bool redirect(const std::string& location);

    const std::string& getHeader(const std::string& key) { return this->m_Headers[key]; };
    void setHeader(const std::string& key, const std::string& value) { this->m_Headers[key] = value; };
    bool removeHeader(const std::string& key) { return this->m_Headers.erase(key) != 0; };

    void setHeaders(const HeadersMap_t& headers);
    const HeadersMap_t& getHeaders() const { return this->m_Headers; };

private:
    std::ostringstream toHttpString();
    bool sendToSocket(const std::string& data);
    void closeSocket() const;
};

#endif // !HTTPRESPONSE_HPP
