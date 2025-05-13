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
        HTTP_2_0 = 20
    };

    inline std::string toString(Version version)
    {
        switch (version)
        {
            case HttpVersion::HTTP_0_9: return "HTTP/0.9";
            case HttpVersion::HTTP_1_0: return "HTTP/1.0";
            case HttpVersion::HTTP_1_1: return "HTTP/1.1";
            case HttpVersion::HTTP_2_0: return "HTTP/2.0";

            default: return std::string();
        };
    };

    inline Version fromString(const std::string& version)
    {
        std::string versionString;
        std::transform(version.begin(), version.end(), std::back_inserter(versionString),
            [](char c) { return toupper(c); });

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
        else {
            throw std::invalid_argument("Unexpected HTTP version");
        };
    };
};

#endif // !HTTPVERSION_HPP
