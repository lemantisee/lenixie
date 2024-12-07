#pragma once

#include <stdint.h>
#include <stm32f1xx.h>

#include "NTPRequest.h"
#include "DateTime.h"
#include "SString.h"

class Wifi;

class RTClock
{
public:
    RTClock() = default;
    void init(Wifi *wifi);
    bool setTime(const DateTime &dateTime);
    bool setTime(uint8_t hours, uint8_t minutes, uint8_t seconds);
    void setTimeZone(int timezone);
    void setNtpServer(const SString<128> &url);

    const DateTime &getTime() const;
    int getTimeZone() const;
    const SString<128> &getNtpServer() const;

    void process();
    void interrupt();
    void syncNtp();

private:
    void syncTime(const SString<128> &ntpServer);
    bool setRtcTime(uint8_t hours, uint8_t minutes, uint8_t seconds);
    bool setRtcDate(uint32_t year, uint8_t month, uint8_t mday, uint8_t wday);
    void updateTime();

    RTC_HandleTypeDef mHandle;
    DateTime mTime;
    NTPRequest mNtp;
    uint32_t mLastNtpSyncTimeMs = 0;
    int mTimezone = 0;
    bool mInited = false;
    SString<128> mNtpUrl = "pool.ntp.org";
};
