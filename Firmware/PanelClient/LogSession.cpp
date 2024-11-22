#include "LogSession.h"

#include "Logger.h"

void LogSession::handle(const PanelMessage &msg)
{
    if (msg.cmd != GetLog) {
        toNext(msg);
        return;
    }

    if (empty()) {
        sendEnd();
        return;
    }

    SString<64> logMsg = popReport();
    if (logMsg.empty()) {
        sendEnd();
        return;
    }

    send(logMsg);
}

SString<64> LogSession::popReport()
{
    if (mCurrentLogIndex >= mLogs.size() - 1) {
        mCurrentLogIndex = -1;
    }

    if (mCurrentLogIndex == -1) {
        SString<128> str = Logger::pop();
        if (str.empty()) {
            return {};
        }

        SString<256> escapedString = escapeString(str);
        mLogs = splitString(escapedString);
        mCurrentLogIndex = 0;
    }

    const SString<48> &token = mLogs[mCurrentLogIndex];
    ++mCurrentLogIndex;

    if (token.empty()) {
        return {};
    }

    const bool end = token.size() < token.capacity();
    return createLogUnit(token, end);
}

bool LogSession::empty() { return Logger::empty() && mCurrentLogIndex == -1; }

void LogSession::sendEnd()
{
    JsonObject j;
    j.add("id", LogEnd);
    send(j.dump());
}

SString<64> LogSession::createLogUnit(const SString<48> &str, bool end) const
{
    JsonObject j;

    j.add("id", end ? LogUnitEnd : LogUnit);
    j.add("d", str.c_str());
    return j.dump();
}

std::array<SString<48>, 6> LogSession::splitString(const SString<256> &str) const
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

SString<256> LogSession::escapeString(const SString<128> &str) const
{
    SString<256> escString;

    for (char c : str) {
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