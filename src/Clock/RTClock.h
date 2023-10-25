#pragma once

#include <stdint.h>
#include <stm32f1xx.h>

#include "NTPHandle.h"

class ESP9266;

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
	void init(ESP9266 *wifi);
	void setTime(uint8_t hours, uint8_t minutes, uint8_t seconds);
	void setTimeZone(uint8_t timezone);
	const Time &getTime();
	void process();
	void syncTime(const char *ntpServer);

private:
	RTC_HandleTypeDef mHandle;
	Time mTime;
	uint8_t mTimezone = 0;
	NTPHandle mNtp;
	uint32_t mLastNtpSyncTime = 0;
};

