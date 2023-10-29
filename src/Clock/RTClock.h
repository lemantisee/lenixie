#pragma once

#include <stdint.h>
#include <stm32f1xx.h>

#include "NTPRequest.h"

class ESP8266;

class RTClock
{
public:
    struct Time
    {
        uint8_t hours = 0;
        uint8_t minutes = 0;
        uint8_t seconds = 0;
    };
    
    RTClock () = default;
    void init(ESP8266 *wifi);
    bool setTime(uint8_t hours, uint8_t minutes, uint8_t seconds);
    void setTimeZone(uint8_t timezone);
    const Time &getTime();
    void process();
    void syncTime(const char *ntpServer);
    void interrupt();

private:
    bool setRtcTime(uint8_t hours, uint8_t minutes, uint8_t seconds);
    bool setRtcDate(uint32_t year, uint8_t month, uint8_t mday, uint8_t wday);
    int8_t calculateDST(uint8_t month, uint8_t monthday, uint8_t weekday, uint8_t hours);
    uint8_t getLastDayOfWeek(uint8_t monthday, uint8_t weekday) const;
    RTC_HandleTypeDef mHandle;
    Time mTime;
    NTPRequest mNtp;
    uint32_t mLastNtpSyncTime = 0;
    int8_t mDstCorrection = 0;
    bool mInited = false;
};

