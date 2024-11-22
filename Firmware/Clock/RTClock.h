#pragma once

#include <stdint.h>
#include <stm32f1xx.h>

#include "NTPRequest.h"
#include "DateTime.h"

class ESP8266;

class RTClock
{
public:
    RTClock() = default;
    void init(ESP8266 *wifi);
    bool setTime(uint8_t hours, uint8_t minutes, uint8_t seconds);
    void setTimeZone(uint8_t timezone);
    const DateTime &getTime() const;
    void process();
    void syncTime(const char *ntpServer);
    void interrupt();

private:
    bool setRtcTime(uint8_t hours, uint8_t minutes, uint8_t seconds);
    bool setRtcDate(uint32_t year, uint8_t month, uint8_t mday, uint8_t wday);
    void updateTime();

    RTC_HandleTypeDef mHandle;
    DateTime mTime;
    NTPRequest mNtp;
    uint32_t mLastNtpSyncTime = 0;
    uint8_t mTimezone = 0;
    bool mInited = false;
};
