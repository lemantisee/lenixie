#include "RTClock.h"

#include <cstdlib>

#include "Wifi.h"
#include "SString.h"
#include "Settings.h"
#include "Logger.h"

namespace {
// constexpr uint32_t ntpPeriodSync = 12 * 60 * 60 * 1000; // 12 hours
constexpr uint32_t ntpPeriodSyncMs = 10 * 60 * 1000; // 10 min
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
RTClock *instance = nullptr;

} // namespace

extern "C" {

void RTC_IRQHandler()
{
    if (instance) {
        instance->interrupt();
    }
}

}

RTClock::RTClock() { instance = this; }

void RTClock::init(Wifi *wifi)
{
    mTimezone = Settings::getTimezone(mTimezone);
    mNtpUrl = Settings::getNtpUrl("");
    
    mHandle.Instance = RTC;
    mHandle.Init.AsynchPrediv = RTC_AUTO_1_SECOND;
    mHandle.Init.OutPut = RTC_OUTPUTSOURCE_NONE;

    HAL_RTC_RegisterCallback(&mHandle, HAL_RTC_MSPINIT_CB_ID, &rtcInit);
    HAL_RTC_RegisterCallback(&mHandle, HAL_RTC_MSPDEINIT_CB_ID, &rtcDeinit);

    HAL_RTC_Init(&mHandle);
    __HAL_RTC_ALARM_ENABLE_IT(&mHandle, RTC_IT_SEC);

    wifi->onConnect([this] { syncTime(mNtpUrl.c_str()); });

    mNtp.init(wifi);
    mInited = true;
}

bool RTClock::setTime(const DateTime &dateTime)
{
    if (!mInited) {
        return false;
    }

    if (!setRtcTime(dateTime.hours, dateTime.minutes, dateTime.seconds)) {
        LOG("Uanble to set time");
        return false;
    }

    if (!setRtcDate(dateTime.year, dateTime.month, dateTime.monthDay, dateTime.weekDay)) {
        LOG("Uanble to set date");
        return false;
    }

    return true;
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

int RTClock::getTimeZone() const { return mTimezone; }

const SString<128> &RTClock::getNtpServer() const
{
    return mNtpUrl;
}

void RTClock::process()
{
    if (!mInited) {
        return;
    }

    if (mLastNtpSyncTimeMs + ntpPeriodSyncMs < HAL_GetTick()) {
        mLastNtpSyncTimeMs = HAL_GetTick();
        syncTime(mNtpUrl.c_str());
    }

    if (logRtcTimeAfterSync) {
        logRtcTimeAfterSync = false;
        LOG("Time: %02i:%02i:%02i", mTime.hours, mTime.minutes, mTime.seconds);
        LOG("Date: %i-%02i-%02i(%i)", mTime.year, mTime.month, mTime.monthDay, mTime.weekDay);
    }
}

void RTClock::syncTime(const SString<128> &ntpServer)
{
    if (ntpServer.empty()) {
        return;
    }

    logRtcTimeAfterSync = true;

    if (auto timestampOpt = mNtp.getTimestamp(ntpServer.c_str())) {
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

    if (mOnTimeChangedCallback) {
        mOnTimeChangedCallback(mTime);
    }
}

void RTClock::setTimeZone(int timezone)
{
    mTimezone = timezone;
    Settings::setTimezone(mTimezone);
}

void RTClock::setNtpServer(const SString<128> &url) 
{
    mNtpUrl = url;
    Settings::setNtpUrl(mNtpUrl);
}

void RTClock::syncNtp() 
{
    syncTime(mNtpUrl.c_str());
}

void RTClock::onTimeChanged(std::function<void(const DateTime &time)> func) 
{
    mOnTimeChangedCallback = func;
}
