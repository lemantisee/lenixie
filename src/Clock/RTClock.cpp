#include "RTClock.h"

#include <cstdlib>

#include "SString.h"
#include "Logger.h"

namespace
{
    constexpr uint32_t ntpPeriodSync = 43200000; // 12 hours
    const char *ntpServer = "0.pool.ntp.org";
    void rtcInit(RTC_HandleTypeDef *hrtc)
    {
        if (hrtc->Instance == RTC) {
            HAL_PWR_EnableBkUpAccess();
            __HAL_RCC_BKP_CLK_ENABLE();
            __HAL_RCC_RTC_ENABLE();

            HAL_NVIC_SetPriority(RTC_IRQn, 0, 0);
            HAL_NVIC_EnableIRQ(RTC_IRQn);
        }
    }

    void rtcDeinit(RTC_HandleTypeDef *hrtc)
    {
        if (hrtc->Instance == RTC) {
            __HAL_RCC_RTC_DISABLE();
        }
    }

    bool logRtcTimeAfterSync = false;

} // namespace


void RTClock::init(ESP8266 *wifi)
{
    mHandle.Instance = RTC;
    mHandle.Init.AsynchPrediv = RTC_AUTO_1_SECOND;
    mHandle.Init.OutPut = RTC_OUTPUTSOURCE_NONE;

    HAL_RTC_RegisterCallback(&mHandle, HAL_RTC_MSPINIT_CB_ID, &rtcInit);
    HAL_RTC_RegisterCallback(&mHandle, HAL_RTC_MSPDEINIT_CB_ID, &rtcDeinit);
    
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

const DateTime &RTClock::getTime()
{
    if (!mInited) {
        return mTime;
    }

    RTC_DateTypeDef date = {0};
    if (HAL_RTC_GetDate(&mHandle, &date, RTC_FORMAT_BIN) != HAL_OK) {
        LOG("Unable to get date from rtc");
        return mTime;
    }

    if (date.WeekDay == 0) {
        date.WeekDay = 7;
    }

    RTC_TimeTypeDef time = {0};
    if (HAL_RTC_GetTime(&mHandle, &time, RTC_FORMAT_BIN) != HAL_OK) {
        LOG("Unable to get time from rtc");
        return mTime;
    }

    mTime.year = date.Year + 2000;
    mTime.month = date.Month - 1;
    mTime.monthDay = date.Date;
    mTime.hours = time.Hours;
    mTime.minutes = time.Minutes;
    mTime.seconds = time.Seconds;

    if (logRtcTimeAfterSync) {
        logRtcTimeAfterSync = false;
        LOG("Time: %02i:%02i:%02i", mTime.hours, mTime.seconds, mTime.minutes);
        LOG("Date: %i-%02i-%02i(%i)", mTime.year, mTime.month, mTime.monthDay, date.WeekDay);
    }

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
    if (mNtp.request(ntpServer)) {
        const DateTime time = mNtp.getTime();
        LOG("Ntp Date: %i-%02i-%02i(%i)", time.year, time.month, time.monthDay, time.weekDay);
        LOG("Ntp Time: %02i:%02i:%02i", time.hours, time.minutes, time.seconds);

        const DateTime localTime = toLocalTime(time);

        LOG("Loc Date: %i-%02i-%02i(%i)", localTime.year, localTime.month, localTime.monthDay, localTime.weekDay);
        LOG("Loc Time: %02i:%02i:%02i", localTime.hours, localTime.minutes, localTime.seconds);

        if (!setRtcTime(localTime.hours, localTime.minutes, localTime.seconds)) {
            LOG("time set fails");
        }

        if (!setRtcDate(localTime.year, localTime.month, localTime.monthDay, localTime.weekDay)) {
            LOG("date set fails");
        }
    }

    logRtcTimeAfterSync = true;
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

int8_t RTClock::calculateDST(uint8_t month, uint8_t monthday, uint8_t weekday, uint8_t hours) const
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

    const uint8_t lastSunday = getLastSunday(monthday, weekday);
    const bool edge = monthday > lastSunday || (monthday == lastSunday && hours >= 3);

    if (month == 9) {
        return edge ? -1 : 0;
    }

    if (month == 2) {
        return edge ? 0 : -1;
    }

    return 0;
}

uint8_t RTClock::getLastSunday(uint8_t monthday, uint8_t weekday) const
{
    // March and October has 31 days
    // we interested in period from 25 day
    // only this days can be in the end of last week in month which has 31 day

    if (monthday < 25) {
        return monthday;
    }

    uint8_t forwardDay = monthday + 7 - weekday;
    if (forwardDay <= 31) {
        return forwardDay;
    }

    return monthday - weekday;
}

DateTime RTClock::toLocalTime(DateTime utcTime) const
{
    DateTime localTime = utcTime;
    const int dstH = calculateDST(utcTime.month, utcTime.monthDay, utcTime.weekDay, utcTime.hours);
    localTime.addDstHour(dstH);

    return localTime;
}

void RTClock::setTimeZone(uint8_t timezone)
{
    mNtp.setTimezone(timezone);
}
