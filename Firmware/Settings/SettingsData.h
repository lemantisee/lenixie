#pragma once

#include <cstring>
#include <cstdint>
#include <limits>

struct SettingsData
{
    enum BoolPosition {
        EnableNtp = 0,
        EnableDND,
    };

    SettingsData();

    bool isNull() const;
    bool getBool(BoolPosition pos) const;
    void setBool(BoolPosition pos, bool state);
    void setNotEmpty();
    void validate();

private:
    uint32_t mEmptyMark = std::numeric_limits<uint32_t>::max();
    uint32_t mBoolArray = 0;

public:
    static const uint16_t stringLength = 128;
    int timezone = 0;
    char ntpUrl[128];
    char wifiSsid[128];
    char wifiPassword[128];
    uint32_t mDndStartHour = 0;
    uint32_t mDndEndHour = 0;
};