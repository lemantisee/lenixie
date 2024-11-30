#pragma once

#include "SString.h"
#include "SettingsData.h"

class Settings
{
public:

    static void init();

    static int getTimezone(int defaultvalue);
    static SString<128> getNtpUrl(const char *defaultValue);
    static SString<128> getWifiSSID(const char *defaultValue);
    static SString<128> getWifiPassword(const char *defaultValue);

    static void setTimezone(int timezone);
    static void setNtpUrl(const SString<128> &url);
    static void setWifiSSID(const SString<128> &ssid);
    static void setWifiPassword(const SString<128> &password);
    

private:
    Settings() = default;

    static Settings &getInstance();
    void readSettings();
    void writeSettings();
    static void copyString(char *dest, const SString<128> &str);

    SettingsData mData;
};
