#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include <unordered_map>
#include <stdexcept>
#include <sstream>

#include "HttpMethod.hpp"
#include "HttpVersion.hpp"

typedef std::unordered_map<std::string, std::string> Map_t;

class HttpRequest
{
private:
    std::string m_Path, m_Body;
    HttpMethod::Method m_Method;
    HttpVersion::Version m_Version;
    Map_t m_Headers;

public:
    HttpRequest(const std::string& data);
    ~HttpRequest() = default;

    std::string getPath() const { return this->m_Path; };
    std::string getBody() const { return this->m_Body; };
    Map_t getHeaders() const { return this->m_Headers; };

    HttpMethod::Method getMethod() const { return this->m_Method; };
    HttpVersion::Version getVersion() const { return this->m_Version; };
};

#endif // !HTTPREQUEST_HPP
