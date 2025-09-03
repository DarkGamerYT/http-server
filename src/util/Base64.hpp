#ifndef BASE64_HPP
#define BASE64_HPP

#include <string>
#include <array>
#include <vector>
#include <span>

namespace Base64 {
    constexpr char CHARACTERS[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    constexpr std::array<uint8_t, 256> REVERSE_TABLE = [] {
        std::array<uint8_t, 256> table{};
        table.fill(0xFF);
        for (int i = 0; i < 64; ++i)
            table[static_cast<uint8_t>(CHARACTERS[i])] = i;
        return table;
    }();

    std::string encode(const std::string& input);
    std::string encode(std::span<const uint8_t> input);

    std::vector<uint8_t> decode(const std::string& input);
    std::vector<uint8_t> decode(std::span<const uint8_t> input);
};

#endif //BASE64_HPP
