#include "RTClock.h"

#include "SString.h"
#include "Logger.h"

namespace
{
    constexpr uint32_t ntpPeriodSync = 43200000;
    const char *ntpServer = "0.pool.ntp.org";
} // namespace


void RTClock::init(ESP8266 *wifi)
{
    mHandle.Instance = RTC;
    mHandle.Init.AsynchPrediv = RTC_AUTO_1_SECOND;
    mHandle.Init.OutPut = RTC_OUTPUTSOURCE_NONE;
    HAL_RTC_Init(&mHandle);
    __HAL_RTC_ALARM_ENABLE_IT(&mHandle, RTC_IT_SEC);

    mNtp.init(wifi);
    mInited = true;
    
    syncTime(ntpServer);
}

bool RTClock::setTime(uint8_t hours, uint8_t minutes, uint8_t seconds)
{
    if (!mInited) {
        return false;
    }

    if (hours > 23) {
        return false;
    }

    return setRtcTime(hours, minutes, seconds);
}

const RTClock::Time &RTClock::getTime()
{
    if (!mInited) {
        return mTime;
    }

    RTC_DateTypeDef data = {0};
    if (HAL_RTC_GetDate(&mHandle, &data, RTC_FORMAT_BIN) != HAL_OK) {
        Logger::log("Unable to get date from rtc");
        return mTime;
    }

    SString<100> str;
    str.append("Date is ").appendNumber(data.Month).append("-").appendNumber(data.Date).append("-").appendNumber(data.WeekDay);
    Logger::log(str.c_str());

    if (data.WeekDay == 0) {
        data.WeekDay = 7;
    }

    RTC_TimeTypeDef time = {0};
    if (HAL_RTC_GetTime(&mHandle, &time, RTC_FORMAT_BIN) != HAL_OK) {
        Logger::log("Unable to get time from rtc");
        return mTime;
    }

    int8_t hours = int8_t(time.Hours) + calculateDST(data.Month - 1, data.Date, data.WeekDay, time.Hours);
    if (hours < 0) {
        hours = 24 - std::abs(hours);
    }

    mTime.seconds = time.Seconds;
    mTime.minutes = time.Minutes;
    mTime.hours = hours;

    return mTime;
}

void RTClock::process()
{
    if (!mInited) {
        return;
    }

    if (mLastNtpSyncTime + ntpPeriodSync < HAL_GetTick()) {
        mLastNtpSyncTime = HAL_GetTick();
        syncTime(ntpServer);
    }
}

void RTClock::syncTime(const char *ntpServer)
{
    if (mNtp.request(ntpServer))
    {
        const NTPRequest::DateTime time = mNtp.getTime();
        setRtcTime(time.hours, time.minutes, time.seconds);
        setRtcDate(time.year, time.month, time.monthDay, time.weekDay);
    }
}

void RTClock::interrupt()
{
    __HAL_RTC_ALARM_CLEAR_FLAG(&mHandle, RTC_FLAG_SEC);
}

bool RTClock::setRtcTime(uint8_t hours, uint8_t minutes, uint8_t seconds)
{
    RTC_TimeTypeDef time;
    time.Hours = hours;
    time.Minutes = minutes;
    time.Seconds = seconds;

    return HAL_RTC_SetTime(&mHandle, &time, RTC_FORMAT_BIN) == HAL_OK;
}

bool RTClock::setRtcDate(uint32_t year, uint8_t month, uint8_t mday, uint8_t wday)
{
    RTC_DateTypeDef data = {0};
    data.WeekDay = wday == 7 ? RTC_WEEKDAY_SUNDAY : wday;

    data.Month = month + 1;
    data.Date = mday;
    data.Year = year - 2000;
    return HAL_RTC_SetDate(&mHandle, &data, RTC_FORMAT_BIN) == HAL_OK;
}

int8_t RTClock::calculateDST(uint8_t month, uint8_t monthday, uint8_t weekday, uint8_t hours)
{
    // For Ukraine
    // set back 1 hour on the last week of October on 3am
    // set forward 1 hour on the last week of March on 4am

    if (month > 9 || month < 2) {
        return -1;
    }

    if (month > 2 && month < 8) {
        return 0;
    }

    if (month == 9) {
        const uint8_t lastSunday = getLastSunday(monthday, weekday);

        if (monthday > lastSunday || (monthday == lastSunday && hours >= 3)) {
            return -1;
        }

        return 0;
    }

    if (month == 2) {
        const uint8_t lastDay = getLastSunday(monthday, weekday);

        if (monthday > lastDay || (monthday == lastDay && hours >= 3)) {
            return 0;
        }

        return -1;
    }

    return 0;
}

uint8_t RTClock::getLastSunday(uint8_t monthday, uint8_t weekday) const
{
    // March and October has 31 days
    // we interested in period from 25 day
    // only this days can be in the end of last week in moth which has 31 day

    if (monthday < 25) {
        return monthday;
    }

    uint8_t forwardDay = monthday + 7 - weekday;
    if (forwardDay <= 31) {
        return forwardDay;
    }

    return monthday - weekday;
}

void RTClock::setTimeZone(uint8_t timezone)
{
    mNtp.setTimezone(timezone);
}
