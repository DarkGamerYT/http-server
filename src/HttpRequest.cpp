#include "HttpRequest.hpp"
HttpRequest::HttpRequest(Socket_t clientSocket, const std::string& data)
{
    this->mClientSocket = clientSocket;

    size_t lpos = 0, rpos = 0;
    rpos = data.find("\r\n", lpos);
    if (rpos == std::string::npos)
    {
        throw std::invalid_argument("Could not find request start line");
    };

    std::string headers, body, startLine = data.substr(lpos, rpos - lpos);

    lpos = rpos + 2;
    rpos = data.find("\r\n\r\n", lpos);
    if (rpos != std::string::npos)
    {
        headers = data.substr(lpos, rpos - lpos);

        lpos = rpos + 4, rpos = data.length();
        if (lpos < rpos)
        {
            body = data.substr(lpos, rpos - lpos);
        };
    };

    // Parsing start line
    std::istringstream iss{ startLine };

    std::string method, path, version;
    iss >> method >> path >> version;

    if (!iss.good() && !iss.eof())
    {
        throw std::invalid_argument("Invalid start line format");
    };

    this->mPath = path;
    this->mMethod = HttpMethod::fromString(method);

    const auto& httpVersion = HttpVersion::fromString(version);
    if (httpVersion != HttpVersion::HTTP_1_1) {
        throw std::logic_error("HTTP version not supported");
    };

    this->mVersion = httpVersion;

    // Parsing headers
    iss.clear();
    iss.str(headers);

    std::string line;
    while (std::getline(iss, line))
    {
        std::string key, value;
        std::istringstream header{ line };

        std::getline(header, key, ':');
        std::getline(header, value);

        std::erase_if(key, [](const char& c) { return std::isspace(c); });
        std::erase_if(value, [](const char& c) { return std::isspace(c); });

        this->mHeaders[key] = value;
    };

    this->mBody = body;
};

std::optional<std::string> HttpRequest::getHeader(const std::string& name) const {
    std::string input{name};
    std::ranges::transform(input, input.begin(),
        [](const unsigned char c){ return std::tolower(c); });

    for (auto& [key, value] : this->mHeaders) {
        std::string kLower{key};
        std::ranges::transform(kLower, kLower.begin(),
            [](const unsigned char c){ return std::tolower(c); });

        if (kLower == input)
            return value;
    };

    return std::nullopt;
};

std::string HttpRequest::getRemoteAddr() const
{
    sockaddr_in addr{};
    socklen_t addrLen = sizeof(addr);

    if (getpeername(this->mClientSocket, reinterpret_cast<sockaddr*>(&addr), &addrLen) == -1)
        return "127.0.0.1";

#if defined(_WIN32)
    char address[INET_ADDRSTRLEN];
    InetNtop(AF_INET, &addr.sin_addr, address, INET_ADDRSTRLEN);

    return address;
#else
    return { inet_ntoa(addr.sin_addr) };
#endif
};