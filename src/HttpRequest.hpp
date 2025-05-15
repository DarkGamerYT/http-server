#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include <unordered_map>
#include <stdexcept>
#include <sstream>

#include "HttpMethod.hpp"
#include "HttpVersion.hpp"

typedef std::unordered_map<std::string, std::string> HeadersMap_t;
class HttpRequest
{
    friend class HttpServer;

private:
    std::string m_Path, m_OriginalPath, m_Body;
    HttpMethod::Method m_Method;
    HttpVersion::Version m_Version;
    HeadersMap_t m_Headers;

public:
    HttpRequest(const std::string& data);
    ~HttpRequest() = default;

    const std::string& getPath() const { return this->m_Path; };
    const std::string& getOriginalPath() const { return this->m_OriginalPath; };
    const std::string& getBody() const { return this->m_Body; };
    const HeadersMap_t& getHeaders() const { return this->m_Headers; };

    HttpMethod::Method getMethod() const { return this->m_Method; };
    HttpVersion::Version getVersion() const { return this->m_Version; };

private:
    void setOriginalPath(const std::string& path) { this->m_OriginalPath = path; };
};

#endif // !HTTPREQUEST_HPP
