#include "HttpResponse.hpp"
#include "HttpServer.hpp"

bool HttpResponse::send(std::string data)
{
    if (!this->mHeaders.contains("Content-Type"))
        this->setHeader("Content-Type", "text/html");

    size_t length = data.length();
    this->setHeader("Content-Length", std::to_string(length));

    if (
        (this->mStatusCode >= 100 && this->mStatusCode < 200)
        || HttpStatus::NoContent == this->mStatusCode
        || HttpStatus::NotModified == this->mStatusCode
    ) {
        this->removeHeader("Content-Type");
        this->removeHeader("Content-Length");
        this->removeHeader("Transfer-Encoding");
        data = "";
    };

    if (HttpStatus::ResetContent == this->mStatusCode) {
        this->removeHeader("Transfer-Encoding");
        data = "";
    };

    std::ostringstream stream = this->toHttpString();
    if (const auto& requestMethod = this->mRequest.getMethod();
        requestMethod != HttpMethod::HEAD &&
        requestMethod != HttpMethod::CONNECT
    ) {
        stream << "\r\n" << data;
    };

    return this->sendToSocket(stream.str());
};

bool HttpResponse::sendStatus(HttpStatus::Code status)
{
    this->mStatusCode = status;
    return this->send("");
};

bool HttpResponse::sendFile(const std::filesystem::path& path)
{
    std::ifstream file(path, std::ios::binary);
    if (!file)
    {
        this->setStatus(HttpStatus::InternalServerError);
        this->setHeader("Content-Type", "text/plain");
        return this->send("Server Error");
    };

    std::ostringstream buffer;
    buffer << file.rdbuf();

    this->setHeader("Content-Type", MimeType::getMimeType(path));
    return this->send(buffer.str());
};

bool HttpResponse::redirect(const std::string& location)
{
    if (this->mStatusCode < 300 || this->mStatusCode >= 400)
        this->mStatusCode = HttpStatus::Found;

    this->setHeader("Location", location);

    std::ostringstream stream = this->toHttpString();
    return this->sendToSocket(stream.str());
}

std::ostringstream HttpResponse::toHttpString()
{
    std::string statusMessage = HttpStatus::toString(this->mStatusCode);

    std::ostringstream stream;
    stream
        << "HTTP/1.1 " << std::to_string(this->mStatusCode)
        << " " << statusMessage << "\r\n";

    for (const auto& [key, value] : this->mHeaders)
        stream << key << ": " << value << "\r\n";
    return stream;
};

void HttpResponse::setHeaders(const std::unordered_map<std::string, std::string>& headers)
{
    for (const auto& [key, value] : headers)
        this->setHeader(key, value);
};

bool HttpResponse::sendToSocket(const std::string& data)
{
    if (true == this->mHeadersSent)
        return false;

    HttpServer::sendToSocket(this->mClientSocket, data);

    this->mHeadersSent = true;
    if (this->mShouldClose)
        this->closeSocket();
    return true;
};

void HttpResponse::closeSocket() const
{
#if defined(_WIN32)
    ::closesocket(this->mClientSocket);
#elif defined(__unix__) || defined(__APPLE__)
    ::close(this->mClientSocket);
#endif
};
