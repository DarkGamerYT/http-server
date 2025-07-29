#include "Base64.hpp"
namespace Base64
{
    // Encoding
    std::string encode(const std::string& input) {
        std::vector<uint8_t> bytes(input.begin(), input.end());
        return Base64::encode(bytes);
    };

    std::string encode(const std::vector<uint8_t>& input) {
        auto bytes = input.data();
        std::size_t size = static_cast<int>(input.size());

        std::string result;
        int i = 0;
        unsigned char a[3];
        unsigned char b[4];

        while (size--) {
            a[i++] = *(bytes++);
            if (i != 3)
                continue;

            b[0] = (a[0] & 0xfc) >> 2;
            b[1] = ((a[0] & 0x03) << 4) + ((a[1] & 0xf0) >> 4);
            b[2] = ((a[1] & 0x0f) << 2) + ((a[2] & 0xc0) >> 6);
            b[3] = a[2] & 0x3f;

            for(i = 0; i < 4; i++)
                result += CHARACTERS[b[i]];

            i = 0;
        };

        if (i) {
            for(int j = i; j < 3; j++)
                a[j] = '\0';

            b[0] = (a[0] & 0xfc) >> 2;
            b[1] = ((a[0] & 0x03) << 4) + ((a[1] & 0xf0) >> 4);
            b[2] = ((a[1] & 0x0f) << 2) + ((a[2] & 0xc0) >> 6);
            b[3] = a[2] & 0x3f;

            for (int j = 0; j < i + 1; j++)
                result += CHARACTERS[b[j]];

            while((i++ < 3))
                result += '=';
        };

        return result;
    };

    // Decoding
    std::vector<uint8_t> decode(const std::string& input) {
        std::vector<uint8_t> bytes(input.begin(), input.end());
        return Base64::decode(bytes);
    };

    std::vector<uint8_t> decode(const std::vector<uint8_t>& input) {
        std::vector<uint8_t> result;
        int i = 0;
        uint8_t a[4], b[3];

        for (uint8_t byte : input) {
            if (REVERSE_TABLE[byte] == 0xFF) // Invalid char
                continue;

            a[i++] = REVERSE_TABLE[byte];
            if (i == 4) {
                b[0] = (a[0] << 2) + ((a[1] & 0x30) >> 4);
                b[1] = ((a[1] & 0xf) << 4) + ((a[2] & 0x3c) >> 2);
                b[2] = ((a[2] & 0x3) << 6) + a[3];

                result.insert(result.end(), b, b + 3);
                i = 0;
            };
        };

        if (i) {
            for (int j = i; j < 4; ++j) a[j] = 0;

            b[0] = (a[0] << 2) + ((a[1] & 0x30) >> 4);
            b[1] = ((a[1] & 0xf) << 4) + ((a[2] & 0x3c) >> 2);
            b[2] = ((a[2] & 0x3) << 6) + a[3];

            for (int j = 0; j < i - 1; ++j)
                result.push_back(b[j]);
        };

        return result;
    };
};