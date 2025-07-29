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
    std::string mPath, mBody;
    HttpMethod::Method mMethod;
    HttpVersion::Version mVersion;
    HeadersMap_t mHeaders;

protected:
    Socket_t mClientSocket;
    std::string mOriginalPath;

public:
    HttpRequest(Socket_t clientSocket, const std::string& data);
    ~HttpRequest() = default;

    const std::string& getPath() const { return this->mPath; };
    const std::string& getOriginalPath() const { return this->mOriginalPath; };
    const std::string& getBody() const { return this->mBody; };
    const HeadersMap_t& getHeaders() const { return this->mHeaders; };

    std::string getRemoteAddr() const;
    HttpMethod::Method getMethod() const { return this->mMethod; };
    HttpVersion::Version getVersion() const { return this->mVersion; };

private:
    void setOriginalPath(const std::string& path) { this->mOriginalPath = path; };
};

#endif // !HTTPREQUEST_HPP
