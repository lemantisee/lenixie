#include "RTClock.h"

#include <libopencm3/stm32/rtc.h>

void RTClock::init(ESP9266 *wifi)
{
	rcc_periph_clock_enable(RCC_PWR);
	rtc_auto_awake(RCC_LSE, 0x7fff);

	mNtp.init(wifi);

	// PWR_BackupAccessCmd(ENABLE);
	// if ((RCC->BDCR & RCC_BDCR_RTCEN) != RCC_BDCR_RTCEN) {
	// 	RCC_BackupResetCmd(ENABLE);
	// 	RCC_BackupResetCmd(DISABLE);
	// 	RCC_LSEConfig(RCC_LSE_ON);
	// 	while ((RCC->BDCR & RCC_BDCR_LSERDY) != RCC_BDCR_LSERDY) {}
	// 	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
	// 	// RTC_SetPrescaler(0x7FFF);
	// 	RCC_RTCCLKCmd(ENABLE);
	// 	RTC_WaitForSynchro();
	// }
}

void RTClock::setTime(uint8_t hours, uint8_t minutes, uint8_t seconds)
{
	hours += mTimezone;
	if (hours > 23) hours -= 24;
	uint32_t gen_seconds = seconds + minutes * 60 + hours * 3600;
	rtc_set_counter_val(gen_seconds);
}

const RTClock::Time &RTClock::getTime()
{
	uint32_t gen_seconds = rtc_get_counter_val();
	if (gen_seconds >= 86400) {
		gen_seconds %= 86400;
	}

	mTime.seconds = gen_seconds % 60;
	mTime.minutes = (gen_seconds % 3600) / 60;
	mTime.hours = gen_seconds / 3600;
	return mTime;
}

void RTClock::syncTime(const char *ntpServer)
{
	if (mNtp.process(ntpServer)) {
		setTime(mNtp.getHours(), mNtp.getMinutes(), mNtp.getSeconds());
	}
}

void RTClock::setTimeZone(uint8_t timezone)
{
	mTimezone = timezone;
}
