#include "LogDump.h"

#include "Logger.h"
#include "JsonObject.h"
#include "MonitorCommand.h"

SString<64> LogDump::popReport()
{
    if (mCurrentLogIndex == -1) {
        SString<128> str = Logger::pop();
        if (str.empty()) {
            {};
        }

        SString<256> escapedString = escapeString(str);
        mLogs = splitString(escapedString);
        mCurrentLogIndex = 0;
    }

    if (mCurrentLogIndex >= mLogs.size() - 1) {
        return {};
    }

    const SString<48> &token = mLogs[mCurrentLogIndex];
    ++mCurrentLogIndex;

    if (token.empty()) {
        return {};
    }

    const bool end = token.size() < token.capacity();
    return createLogUnit(token, end);
}

bool LogDump::empty() { return Logger::empty() && mCurrentLogIndex == -1; }

SString<64> LogDump::createLogUnit(const SString<48> &str, bool end) const
{
    JsonObject j;

    j.add("id", end ? LogUnitEnd : LogUnit);
    j.add("d", str.c_str());
    return j.dump();
}

std::array<SString<48>, 6> LogDump::splitString(const SString<256> &str) const
{
    const size_t tokenCapacity = 48;
    size_t strSize = str.size();
    std::array<SString<48>, 6> strings;

    const char *ptr = str.c_str();
    for (SString<48> &token : strings) {
        if (strSize == 0) {
            break;
        }

        const size_t sizeToCopy = std::min(strSize, tokenCapacity);

        token = SString<48>(ptr, sizeToCopy);
        strSize -= sizeToCopy;
        ptr += sizeToCopy;
    }

    return strings;
}

SString<256> LogDump::escapeString(const SString<128> &str) const
{
    SString<256> escString;

    for (char c: str) {
        if (c == 0) {
            break;
        }

        if (c == '"') {
            escString += '\\';
        }

        if (c > 31 && c < 127) {
            escString += c;
            continue;
        }

        const SString<256> hex = Logger::format("<0x%02X>", c);
        escString.append(hex.c_str());
    }

    return escString;
}