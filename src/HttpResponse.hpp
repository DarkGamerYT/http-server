#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include <fstream>

#include "HttpRequest.hpp"
#include "util/HttpStatus.hpp"
#include "util/MimeType.hpp"

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
    bool sendFile(const std::filesystem::path& path);
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
