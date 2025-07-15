#include "HttpResponse.hpp"
#include <iostream>

bool HttpResponse::send(std::string data)
{
    if (!this->m_Headers.contains("Content-Type"))
        this->setHeader("Content-Type", "text/html");

    size_t length = data.length();
    this->setHeader("Content-Length", std::to_string(length));

    if (
        (this->m_StatusCode >= 100 && this->m_StatusCode < 200)
        || HttpStatus::NoContent == this->m_StatusCode
        || HttpStatus::NotModified == this->m_StatusCode
    ) {
        this->removeHeader("Content-Type");
        this->removeHeader("Content-Length");
        this->removeHeader("Transfer-Encoding");
        data = "";
    };

    if (HttpStatus::ResetContent == this->m_StatusCode) {
        this->removeHeader("Transfer-Encoding");
        data = "";
    };

    std::ostringstream stream = this->toHttpString();
    const auto& requestMethod = this->m_Request.getMethod();
    if (
        requestMethod != HttpMethod::HEAD &&
        requestMethod != HttpMethod::CONNECT
    ) {
        stream << "\r\n" << data;
    };

    return this->sendToSocket(stream.str());
};

bool HttpResponse::sendStatus(HttpStatus::Code status)
{
    this->m_StatusCode = status;
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
    if (this->m_StatusCode < 300 || this->m_StatusCode >= 400)
        this->m_StatusCode = HttpStatus::Found;

    this->setHeader("Location", location);

    std::ostringstream stream = this->toHttpString();
    return this->sendToSocket(stream.str());
}

std::ostringstream HttpResponse::toHttpString()
{
    std::string statusMessage = HttpStatus::toString(this->m_StatusCode);

    std::ostringstream stream;
    stream
        << "HTTP/1.1 " << std::to_string(this->m_StatusCode)
        << " " << statusMessage << "\r\n";

    for (const auto& [key, value] : this->m_Headers)
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
    if (true == this->m_HeadersSent)
        return false;

#if defined(_WIN32)
    long totalBytesSent = 0;
    while (totalBytesSent < data.size())
    {
        int bytesSent = ::send(this->m_ClientSocket, data.c_str(), static_cast<int>(data.size()), 0);
        if (bytesSent < 0)
            break;

        totalBytesSent += bytesSent;
    };
#elif defined(__unix__) || defined(__APPLE__)
    long bytesSent = ::write(this->m_ClientSocket, data.c_str(), data.size());
#endif

    this->m_HeadersSent = true;
    this->closeSocket();
    return true;
};

void HttpResponse::closeSocket() const
{
#if defined(_WIN32)
    ::closesocket(this->m_ClientSocket);
#elif defined(__unix__) || defined(__APPLE__)
    ::close(this->m_ClientSocket);
#endif
};
