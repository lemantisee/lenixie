#include "RTClock.h"

#include <cstdlib>

#include "SString.h"
#include "Logger.h"

namespace {
constexpr uint32_t ntpPeriodSync = 43200000; // 12 hours
const char *ntpServer = "pool.ntp.org";
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

const DateTime &RTClock::getTime() const
{
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
    logRtcTimeAfterSync = true;

    if (auto timestampOpt = mNtp.getTimestamp(ntpServer)) {
        const DateTime time = DateTime::fromTimestamp(*timestampOpt + mTimezone * 60 * 60);
        LOG("Ntp Date: %i-%02i-%02i(%i)", time.year, time.month, time.monthDay, time.weekDay);
        LOG("Ntp Time: %02i:%02i:%02i", time.hours, time.minutes, time.seconds);

        const DateTime localTime = DateTime::localDatetime(*timestampOpt, mTimezone);
        if (localTime.isNull()) {
            LOG("datetim convertion fails");
            return;
        }

        LOG("Loc Date: %i-%02i-%02i(%i)", localTime.year, localTime.month, localTime.monthDay,
            localTime.weekDay);

        LOG("Loc Time: %02i:%02i:%02i", localTime.hours, localTime.minutes, localTime.seconds);

        if (!setRtcTime(localTime.hours, localTime.minutes, localTime.seconds)) {
            LOG("time set fails");
        }

        if (!setRtcDate(localTime.year, localTime.month, localTime.monthDay, localTime.weekDay)) {
            LOG("date set fails");
        }
    }
}

void RTClock::interrupt()
{
    if (!mInited) {
        return;
    }

    __HAL_RTC_ALARM_CLEAR_FLAG(&mHandle, RTC_FLAG_SEC);
    updateTime();
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

void RTClock::updateTime()
{
    RTC_DateTypeDef date = {0};
    if (HAL_RTC_GetDate(&mHandle, &date, RTC_FORMAT_BIN) != HAL_OK) {
        LOG("Unable to get date from rtc");
        return;
    }

    if (date.WeekDay == 0) {
        date.WeekDay = 7;
    }

    RTC_TimeTypeDef time = {0};
    if (HAL_RTC_GetTime(&mHandle, &time, RTC_FORMAT_BIN) != HAL_OK) {
        LOG("Unable to get time from rtc");
        return;
    }

    mTime.year = date.Year + 2000;
    mTime.month = date.Month - 1;
    mTime.monthDay = date.Date;
    mTime.hours = time.Hours;
    mTime.minutes = time.Minutes;
    mTime.seconds = time.Seconds;

    if (logRtcTimeAfterSync) {
        logRtcTimeAfterSync = false;
        LOG("Time: %02i:%02i:%02i", mTime.hours, mTime.minutes, mTime.seconds);
        LOG("Date: %i-%02i-%02i(%i)", mTime.year, mTime.month, mTime.monthDay, date.WeekDay);
    }
}

void RTClock::setTimeZone(uint8_t timezone) { mTimezone = timezone; }
