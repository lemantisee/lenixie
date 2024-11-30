#pragma once

#include <cstring>
#include <cstdint>
#include <limits>

struct SettingsData
{
    enum BoolPosition {
        EnableNtp = 0,
    };

    SettingsData();

    bool empty() { return mEmptyMark == std::numeric_limits<uint32_t>::max(); }
    bool getBool(BoolPosition pos) const;
    void setBool(BoolPosition pos, bool state);
    void setNotEmpty();

private:
    uint32_t mEmptyMark = std::numeric_limits<uint32_t>::max();
    uint32_t mBoolArray = 0;

public:
    static const uint16_t stringLength = 128;
    int timezone = 0;
    char ntpUrl[128];
    char wifiSsid[128];
    char wifiPassword[128];
};