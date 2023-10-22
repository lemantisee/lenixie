#pragma once

#include <stdint.h>

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
	void init();
	void setTime(uint8_t hours, uint8_t minutes, uint8_t seconds);
	void setTimeZone(uint8_t timezone);
	const Time &getTime();

private:
	Time mTime;
	uint8_t mTimezone = 0;
};

