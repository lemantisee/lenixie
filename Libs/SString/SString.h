#pragma once

#include <array>
#include <cstring>
#include <cstdio>
#include <optional>
#include <cstdint>
#include <cstdlib>

template<uint32_t N>
class SString
{
public:
    using Buffer = std::array<char, N + 1>;

    SString() = default;
    SString(const char *data, uint32_t size)
    {
        size_t sizeToCopy = std::min<size_t>(capacity(), size);
        std::memcpy(mBuffer.data(), data, sizeToCopy);
        mCurrentByte = sizeToCopy;
    }

    SString(const char *str) { *this = str; }

    SString(const SString &rvl) { *this = rvl; }

    SString(SString &&rvl) { *this = std::move(rvl); }

    SString &operator=(SString &&rvl)
    {
        if (this == &rvl) {
            return *this;
        }
        mBuffer = std::move(rvl.mBuffer);
        mCurrentByte = rvl.mCurrentByte;

        rvl.mCurrentByte = 0;
        rvl.mBuffer.fill(0);

        return *this;
    }

    SString &operator=(const SString &rvl)
    {
        if (this == &rvl) {
            return *this;
        }

        mBuffer = rvl.mBuffer;
        mCurrentByte = rvl.mCurrentByte;

        return *this;
    }

    SString &operator=(const char *str)
    {
        if (mBuffer.data() == str) {
            return *this;
        }

        clear();
        append(str);
        return *this;
    }

    char &operator[](uint32_t index) { return mBuffer[index]; }

    SString &operator+(const SString &rvl)
    {
        if (size() + rvl.size() >= capacity()) {
            return *this;
        }

        std::memcpy(mBuffer.data() + mCurrentByte, rvl.mBuffer.data(), rvl.mCurrentByte);

        mCurrentByte += rvl.mCurrentByte;

        return *this;
    }

    void operator+=(char c) { append(c); }

    void operator+=(const char *str) { append(str); }

    bool operator==(const char *str) const { return std::strcmp(str, c_str()) == 0; }

    SString &append(char c)
    {
        if (mCurrentByte <= capacity() - 1) {
            mBuffer[mCurrentByte] = c;
            ++mCurrentByte;
        }

        return *this;
    }

    SString &append(const char *str)
    {
        const uint32_t size = strlen(str);
        if (mCurrentByte + size - 1 > capacity() - 1) {
            return *this;
        }

        for (uint32_t i = 0; i < size; ++i) {
            append(char(str[i]));
        }

        return *this;
    }

    SString &append(uint16_t c)
    {
        const uint8_t low8bits = c & 0xFF;
        if (c < 255) {
            return append(char(low8bits));
        }

        const uint8_t high8bits = (c >> 8) & 0xFF;
        return append(char(low8bits)) && append(char(high8bits));
    }

    SString &append(const char *data, uint32_t size)
    {
        if (mCurrentByte + size - 1 > capacity() - 1) {
            return *this;
        }

        for (uint32_t i = 0; i < size; ++i) {
            append(char(data[i]));
        }

        return *this;
    }

    SString &append(const uint8_t *data, uint32_t size)
    {
        return append(reinterpret_cast<const char *>(data), size);
    }

    SString &appendNumber(int value)
    {
        return appendNumber(value, "%i");
    }

    SString &appendNumber(int value, const char *format)
    {
        int size = std::snprintf(nullptr, 0, format, value) + 1; // Extra space for '\0'
        if (size > 0 || size + mCurrentByte < capacity()) {
            snprintf(mBuffer.data() + mCurrentByte, size, format, value);
            mCurrentByte += size - 1;
        }

        return *this;
    }

    void clear()
    {
        mBuffer.fill(0);
        mCurrentByte = 0;
    }

    char pop()
    {
        char c = mBuffer[mCurrentByte - 1];
        mBuffer[mCurrentByte - 1] = 0;
        --mCurrentByte;
        return c;
    }

    char back() const { return mBuffer[mCurrentByte - 1]; }

    void resize(uint32_t size)
    {
        mCurrentByte = size;
        std::fill(mBuffer.begin() + mCurrentByte, mBuffer.end(), 0);
    }

    const char *c_str() const { return mBuffer.data(); }

    char *data() { return mBuffer.data(); }

    uint32_t capacity() const { return mBuffer.size() - 1; }

    uint32_t size() const { return mCurrentByte; }

    bool empty() const { return mCurrentByte == 0; }

    bool contains(const char *substr) const
    {
        return std::strstr(mBuffer.data(), substr) != nullptr;
    }

    std::optional<uint32_t> find(const char *substr, uint32_t fromPos = 0) const
    {
        if (fromPos >= mBuffer.size()) {
            return std::nullopt;
        }

        const char *mBufferStr = mBuffer.data() + fromPos;
        if (const char *ptr = strstr(mBufferStr, substr)) {
            return uint32_t(ptr - mBuffer.data());
        }

        return std::nullopt;
    }

    int toInt() const { return std::atoi(mBuffer.data()); }

    void removeSymbol(char symbol)
    {
        for (int i = 0; i < mCurrentByte; ++i) {
            char c = mBuffer[i];
            if (c != symbol) {
                continue;
            }

            std::memmove(&mBuffer[i], &mBuffer[i + 1], mCurrentByte - i - 1);

            --mCurrentByte;
            --i;
            mBuffer[mCurrentByte] = 0;
        }
    }

    Buffer::const_iterator begin() const { return mBuffer.cbegin(); }
    Buffer::const_iterator end() const { return mBuffer.cend(); }

private:
    uint32_t mCurrentByte = 0;
    Buffer mBuffer = {};
};
