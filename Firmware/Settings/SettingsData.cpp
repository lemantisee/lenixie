#include "SettingsData.h"

SettingsData::SettingsData()
{
    std::memset(ntpUrl, 0, stringLength);
    std::memset(wifiSsid, 0, stringLength);
    std::memset(wifiPassword, 0, stringLength);
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
