#pragma once

#include <array>
#include <cstdio>
#include <cstring>

#include "SString.h"
#include "RingBuffer.h"

#define FILENAME strrchr("/" __FILE__, '/') + 1

#define LOG(msg, ...) Logger::log(Logger::Info, FILENAME, __LINE__, msg, ##__VA_ARGS__)
#define LOG_ERROR(msg, ...) Logger::log(Logger::Error, FILENAME, __LINE__, msg, ##__VA_ARGS__)

class Logger
{
public:
    enum Type { Info, Error };
    using String128 = SString<128>;
    using String256 = SString<256>;

    template<typename... Args>
    static void log(Type type, const char *file, int line, const char *fmt, Args... args)
    {
        if (!getInstance().mEnabled) {
            return;
        }

        String256 header = getInstance().createHeader(file, line);
        String256 &str = getInstance().stringFormat(fmt, args...);
        if (str.size() + header.size() + 1 >= str.capacity()) {
            return;
        }

        str = header + str;

        if (str.size() <= getInstance().mBufferStringSize) {
            getInstance().mBuffer.append(String128(str.c_str(), str.size()));
        }
    }

    static void enable(bool state) { getInstance().mEnabled = state; }

    static String128 pop()
    {
        if (!getInstance().mEnabled) {
            return {};
        }

        if (getInstance().mBuffer.empty()) {
            return {};
        }

        return getInstance().mBuffer.pop();
    }

    static bool empty() { return getInstance().mBuffer.empty(); }

    template<typename... Args>
    static String256 format(const char *fmt, Args... args)
    {
        return getInstance().stringFormat(fmt, args...);
    }

private:
    Logger() = default;
    ~Logger() = default;

    static Logger &getInstance()
    {
        static Logger logger;
        return logger;
    }

    template<typename... Args>
    String256 &stringFormat(const char *fmt, Args... args)
    {
        mString.clear();
        int size = std::snprintf(nullptr, 0, fmt, args...) + 1; // Extra space for '\0'
        if (size > 0 || size < mString.capacity()) {
            mString.resize(size - 1);
            std::snprintf(mString.data(), size, fmt, args...);
        }

        return mString;
    }

    String256 createHeader(const char *file, int line) const
    {
        String256 str;
        str.append("[").append(file).append(":").appendNumber(line).append("]");
        return str;
    }

    const uint8_t mBufferStringSize = 128;

    RingBuffer<48, 128> mBuffer;
    String256 mString;
    bool mEnabled = true;
};
