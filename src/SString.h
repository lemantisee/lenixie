#pragma once

#include <cstdint>
#include <cstring>
#include <array>

template <uint32_t N>
class SString
{
public:
    SString() = default;

    void append(const char *str) {
        std::strcat(mBuffer.data(), str);
    }

    void appendNumber(uint32_t value) {
        std::array<char, 11> valueBuffer;
        std::sprintf(valueBuffer.data(), "%d", int(value));
        std::strcat(mBuffer.data(), valueBuffer.data());
    }

    const char *c_str() const {
        return mBuffer.data();
    }

private:
    std::array<char, N> mBuffer = {};
};
