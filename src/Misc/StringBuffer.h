#pragma once

#include <array>
#include <cstring>

template <uint32_t N>
class StringBuffer
{
public:
    bool append(char c)
    {
        if (mCurrentByte < mBuffer.size() - 1)
        {
            mBuffer[mCurrentByte] = c;
            ++mCurrentByte;
            return true;
        }

        return false;
    }

    bool append(uint16_t c)
    {
        const uint8_t low8bits = c & 0xFF;
        if (c < 255)
        {
            return append(char(low8bits));
        }

        const uint8_t high8bits = (c >> 8) & 0xFF;
        return append(char(low8bits)) && append(char(high8bits));
        return append(char(high8bits)) && append(char(low8bits));
    }

    bool append(char *data, uint32_t size)
    {
        if (mCurrentByte + size >= mBuffer.size() - 1)
        {
            return false;
        }

        for (uint32_t i = 0; i < size; ++i) {
            append(char(data[i]));
        }

        return true;
    }

    void clear()
    {
        mBuffer.fill(0);
        mCurrentByte = 0;
    }

    const char *c_str() const
    {
        return mBuffer.data();
    }

    char *data()
    {
        return mBuffer.data();
    }

    uint32_t size() const {
        return mBuffer.size();
    }

    uint32_t capacity() const {
        return mCurrentByte;
    }

    bool contains(const char *substr) const {
        return std::strstr(mBuffer.data(), substr) != nullptr;
    }

private:
    uint32_t mCurrentByte = 0;
    std::array<char, N> mBuffer = {};
};
