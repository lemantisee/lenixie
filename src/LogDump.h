#pragma once

#include "SString.h"

class LogDump
{
public:
    SString<64> popReport();
    bool empty();

private:
    SString<64> createLogUnit(const SString<48> &str, bool end) const;
    std::array<SString<48>, 6> splitString(const SString<256> &str) const;
    SString<256> escapeString(const SString<128> &str) const;
    std::array<SString<48>, 6> mLogs;
    int mCurrentLogIndex = -1;
};
