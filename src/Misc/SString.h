#pragma once

#include <cstdint>
#include <cstring>
#include <array>

template <uint32_t N>
class SString
{
public:
    SString() = default;

    SString &operator=(const char *str) {
        mBuffer.fill(0);
        if (std::strlen(str) > mBuffer.size() - 1) {
            return *this;
        }

        std::strcpy(mBuffer.data(), str);
        return *this;
    }

    operator const char *() const {
        return mBuffer.data();
    }

    SString &append(const char *str) {
        std::strcat(mBuffer.data(), str);
        return *this;
    }

    SString &appendNumber(uint32_t value) {
        std::array<char, 11> valueBuffer;
        std::sprintf(valueBuffer.data(), "%d", int(value));
        std::strcat(mBuffer.data(), valueBuffer.data());
        return *this;
    }

    const char *c_str() const {
        return mBuffer.data();
    }

private:
    std::array<char, N> mBuffer = {};
};
