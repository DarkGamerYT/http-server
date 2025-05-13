#include "HttpResponse.hpp"

bool HttpResponse::send(std::string data)
{
    if (this->m_Headers.find("Content-Type") == this->m_Headers.end())
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
    bool bIsSuccessful = (this->m_StatusCode >= 200 && this->m_StatusCode < 300);
    if (requestMethod != HttpMethod::HEAD &&
        (requestMethod != HttpMethod::CONNECT && bIsSuccessful))
        stream << "\r\n" << data;

    return this->sendToSocket(stream.str());
};

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

bool HttpResponse::sendToSocket(const std::string& data)
{
    if (true == this->m_HeadersSent)
        return false;

#if defined(_WIN32)
    int bytesSent;
    long totalBytesSent = 0;
    while (totalBytesSent < data.size())
    {
        bytesSent = ::send(this->m_ClientSocket, data.c_str(), data.size(), 0);
        if (bytesSent < 0)
            break;

        totalBytesSent += bytesSent;
    };
#elif defined(__unix__)
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
#elif defined(__unix__)
    ::close(this->m_ClientSocket);
#endif
};