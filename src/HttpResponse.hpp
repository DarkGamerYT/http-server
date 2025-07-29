#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include <fstream>

#include "HttpRequest.hpp"
#include "util/HttpStatus.hpp"
#include "util/MimeType.hpp"

class HttpResponse
{
private:
    bool mHeadersSent = false;
    HttpStatus::Code mStatusCode = HttpStatus::OK;
    HeadersMap_t mHeaders;

protected:
    Socket_t mClientSocket;
    HttpRequest mRequest;
    bool mShouldClose = true;

public:
    HttpResponse(Socket_t clientSocket, const HttpRequest& req, bool shouldClose = true) :
        mClientSocket(clientSocket),
        mRequest(req),
        mShouldClose(shouldClose) {};

    HttpResponse& setStatus(HttpStatus::Code status) { this->mStatusCode = status; return *this; };

    bool send(std::string data = "");
    bool sendStatus(HttpStatus::Code status);
    bool sendFile(const std::filesystem::path& path);
    bool redirect(const std::string& location);

    const std::string& getHeader(const std::string& key) { return this->mHeaders[key]; };
    void setHeader(const std::string& key, const std::string& value) { this->mHeaders[key] = value; };
    bool removeHeader(const std::string& key) { return this->mHeaders.erase(key) != 0; };

    void setHeaders(const HeadersMap_t& headers);
    const HeadersMap_t& getHeaders() const { return this->mHeaders; };

private:
    std::ostringstream toHttpString();
    bool sendToSocket(const std::string& data);
    void closeSocket() const;
};

#endif // !HTTPRESPONSE_HPP
