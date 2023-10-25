#include "RTClock.h"

namespace
{
	constexpr uint32_t ntpPeriodSync = 43200000;
	const char *ntpServer = "0.pool.ntp.org";
} // namespace


void RTClock::init(ESP9266 *wifi)
{
	mHandle.Instance = RTC;
	mHandle.Init.AsynchPrediv = RTC_AUTO_1_SECOND;
	mHandle.Init.OutPut = RTC_OUTPUTSOURCE_NONE;
	HAL_RTC_Init(&mHandle);

	mNtp.init(wifi);
	syncTime(ntpServer);
}

void RTClock::setTime(uint8_t hours, uint8_t minutes, uint8_t seconds)
{
	hours += mTimezone;
	if (hours > 23){
		hours -= 24;
	}

	RTC_TimeTypeDef time;
	time.Hours = hours;
    time.Minutes = minutes;
    time.Seconds = seconds;

	HAL_RTC_SetTime(&mHandle, &time, RTC_FORMAT_BIN);
}

const RTClock::Time &RTClock::getTime()
{
	RTC_TimeTypeDef time;
	HAL_RTC_GetTime(&mHandle, &time, RTC_FORMAT_BIN);

	mTime.seconds = time.Seconds;
	mTime.minutes = time.Minutes;
	mTime.hours = time.Hours;
	return mTime;
}

void RTClock::process()
{
	if (mLastNtpSyncTime + ntpPeriodSync > HAL_GetTick()) {
		mLastNtpSyncTime = HAL_GetTick();
		syncTime(ntpServer);
	}
}

void RTClock::syncTime(const char *ntpServer)
{
	if (mNtp.process(ntpServer))
	{
		setTime(mNtp.getHours(), mNtp.getMinutes(), mNtp.getSeconds());
	}
}

void RTClock::setTimeZone(uint8_t timezone)
{
	mTimezone = timezone;
}
