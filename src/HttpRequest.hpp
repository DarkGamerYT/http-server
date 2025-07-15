#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include <unordered_map>
#include <stdexcept>
#include <sstream>

#if defined(_WIN32)
    #include <WinSock2.h>
    #include <ws2tcpip.h>
#elif defined(__unix__) || defined(__APPLE__)
    #include <unistd.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
#endif

#if defined(_WIN32)
    typedef SOCKET Socket_t;
#elif defined(__unix__) || defined(__APPLE__)
    typedef int Socket_t;
#endif

#include "util/HttpMethod.hpp"
#include "util/HttpVersion.hpp"

typedef std::unordered_map<std::string, std::string> HeadersMap_t;
class HttpRequest
{
    friend class HttpServer;

private:
    std::string m_Path, m_Body;
    HttpMethod::Method m_Method;
    HttpVersion::Version m_Version;
    HeadersMap_t m_Headers;

protected:
    Socket_t m_ClientSocket;
    std::string m_OriginalPath;

public:
    HttpRequest(Socket_t clientSocket, const std::string& data);
    ~HttpRequest() = default;

    const std::string& getPath() const { return this->m_Path; };
    const std::string& getOriginalPath() const { return this->m_OriginalPath; };
    const std::string& getBody() const { return this->m_Body; };
    const HeadersMap_t& getHeaders() const { return this->m_Headers; };

    std::string getRemoteAddr() const;
    HttpMethod::Method getMethod() const { return this->m_Method; };
    HttpVersion::Version getVersion() const { return this->m_Version; };

private:
    void setOriginalPath(const std::string& path) { this->m_OriginalPath = path; };
};

#endif // !HTTPREQUEST_HPP
