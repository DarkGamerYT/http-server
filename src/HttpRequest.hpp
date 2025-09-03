#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include <unordered_map>
#include <sstream>

#include "util/HttpMethod.hpp"
#include "util/HttpVersion.hpp"

#include "Common.hpp"

typedef std::unordered_map<std::string, std::string> HeadersMap_t;
class HttpRequest
{
    friend class HttpServer;

private:
    std::string mPath{};
    std::string mBody{};
    HttpMethod::Method mMethod{ HttpMethod::GET };
    HttpVersion::Version mVersion{ HttpVersion::HTTP_1_1 };
    HeadersMap_t mHeaders{};

protected:
    Socket_t mClientSocket{ 0 };
    std::string mOriginalPath{};

public:
    HttpRequest(Socket_t clientSocket, const std::string& data);
    ~HttpRequest() = default;

    [[nodiscard]] const std::string& getPath() const { return this->mPath; };
    [[nodiscard]] const std::string& getOriginalPath() const { return this->mOriginalPath; };
    [[nodiscard]] const std::string& getBody() const { return this->mBody; };

    [[nodiscard]] const HeadersMap_t& getHeaders() const { return this->mHeaders; };
    std::optional<std::string> getHeader(const std::string& name) const;

    [[nodiscard]] std::string getRemoteAddr() const;
    [[nodiscard]] HttpMethod::Method getMethod() const { return this->mMethod; };
    [[nodiscard]] HttpVersion::Version getVersion() const { return this->mVersion; };

private:
    void setOriginalPath(const std::string& path) { this->mOriginalPath = path; };
};

#endif // !HTTPREQUEST_HPP
