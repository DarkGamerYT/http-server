#include "HttpRequest.hpp"
#include <iostream>
HttpRequest::HttpRequest(const std::string& data)
{
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

    this->m_Path = path;
    this->m_Method = HttpMethod::fromString(method);

    const auto& httpVersion = HttpVersion::fromString(version);
    if (httpVersion != HttpVersion::HTTP_1_1) {
        throw std::logic_error("HTTP version not supported");
    };

    this->m_Version = httpVersion;

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

        key.erase(std::remove_if(key.begin(), key.end(),
            [](char c) { return std::isspace(c); }), key.end());

        value.erase(std::remove_if(value.begin(), value.end(),
            [](char c) { return std::isspace(c); }), value.end());

        this->m_Headers[key] = value;
    };

    this->m_Body = body;
};