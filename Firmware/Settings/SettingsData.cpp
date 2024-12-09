#include "SettingsData.h"

SettingsData::SettingsData()
{
    std::memset(ntpUrl, 0, stringLength);
    std::memset(wifiSsid, 0, stringLength);
    std::memset(wifiPassword, 0, stringLength);

    setBool(EnableNtp, true);
    setBool(EnableDND, false);
}

bool SettingsData::isNull() const
{
    // empty page filled with 1
    // check is first 4 btes are 0xFF that means this is fresh start and has no settings
    return mEmptyMark == std::numeric_limits<uint32_t>::max();
}

bool SettingsData::getBool(BoolPosition pos) const { return mBoolArray & (1 << pos); }

void SettingsData::setBool(BoolPosition pos, bool state) 
{
    if(!state) {
        mBoolArray = mBoolArray & ~(1 << pos);
    } else {
        mBoolArray = mBoolArray | (1 << pos);
    }
}

void SettingsData::setNotEmpty() 
{
    mEmptyMark = 0;
}
