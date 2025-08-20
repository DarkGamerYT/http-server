#ifndef HTTPVERSION_HPP
#define HTTPVERSION_HPP

#include <string>
#include <iterator>
#include <algorithm>
#include <stdexcept>

namespace HttpVersion
{
    enum Version {
        HTTP_0_9 = 9,
        HTTP_1_0 = 10,
        HTTP_1_1 = 11,
        HTTP_2_0 = 20,
        HTTP_3_0 = 30
    };

    inline std::string toString(const Version version)
    {
        switch (version)
        {
            case HttpVersion::HTTP_0_9: return "HTTP/0.9";
            case HttpVersion::HTTP_1_0: return "HTTP/1.0";
            case HttpVersion::HTTP_1_1: return "HTTP/1.1";
            case HttpVersion::HTTP_2_0: return "HTTP/2.0";
            case HttpVersion::HTTP_3_0: return "HTTP/3.0";

            default: return {};
        };
    };

    inline Version fromString(const std::string& version)
    {
        std::string versionString;
        std::ranges::transform(version, std::back_inserter(versionString),
            [](const char& c) { return toupper(c); });

        if (versionString == "HTTP/0.9") {
            return HttpVersion::HTTP_0_9;
        }
        else if (versionString == "HTTP/1.0") {
            return HttpVersion::HTTP_1_0;
        }
        else if (versionString == "HTTP/1.1") {
            return HttpVersion::HTTP_1_1;
        }
        else if (versionString == "HTTP/2" || versionString == "HTTP/2.0") {
            return HttpVersion::HTTP_2_0;
        }
        else if (versionString == "HTTP/3" || versionString == "HTTP/3.0") {
            return HttpVersion::HTTP_3_0;
        }
        else {
            throw std::invalid_argument("Unexpected HTTP version");
        };
    };
};

#endif // !HTTPVERSION_HPP
