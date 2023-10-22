#include "NTPHandle.h"

#include "ESP9266.h"
#include <cstring>
// #include "Logger.h"

const int mins_in_hour = 60;
const int secs_to_min = 60;
uint8_t ntpRequset[48] = { 0x1B, 0, 0, 0, 0, 0, 0, 0, 0 };

void NTPHandle::delay(uint32_t ticks) const {
	for (uint32_t i = 0; i < ticks; i++) {
		asm("nop");
	}
}

void NTPHandle::init(ESP9266 *wifi) {
	mWifi = wifi;
	std::memset(&mNtpAnswer, 0, 48); 
}

bool NTPHandle::getNtpRequest() {
	mWifi->sendUDPpacket((char *)ntpRequset, 48);
	mWifi->clearBuffer();
	delay(100000);
	uint8_t *ptr = mWifi->getData(48);
	delay(100000);
	if(ptr != nullptr)
	{
		std::memcpy(&mNtpAnswer, ptr, sizeof(mNtpAnswer));
		mSecondsFromStart = (htonl(mNtpAnswer.txTm_s) - NTP_TIMESTAMP_DELTA) % 86400;
		mWifi->clearBuffer();
		std::memset(&mNtpAnswer, 0, 48);
		return true;
	}
	else
	{
		return false;
	}
}

bool NTPHandle::getTime() {
	if (mSecondsFromStart) {
		mSeconds = mSecondsFromStart % secs_to_min;
		mMinutes = mSecondsFromStart / secs_to_min % mins_in_hour;
		mHours = (mSecondsFromStart / secs_to_min / mins_in_hour);
		mSecondsFromStart = 0;
		return true;
	}
	return false;
}

uint8_t NTPHandle::getHours() {
	return mHours;
}

uint8_t NTPHandle::getMinutes() {
	return mMinutes;
}

uint8_t NTPHandle::getSeconds() {
	return mSeconds;
}

bool NTPHandle::process(const char *server){
    setServer(server, 123);
    for (int i = 0; i < 2; i++) {
        getNtpRequest();
        delay(6000000);
        if (getTime()) {
			return true;
        }
    }
	return false;
}

void NTPHandle::setServer(const char *host, uint16_t port) {
	mHost = (char *)host;
	mPort = port;
	mWifi->connectToServerUDP(mHost, mPort);
}

uint32_t NTPHandle::htonl(uint32_t val) const
{
	val = ((((val) >> 24) & 0xff) | (((val) >> 8) & 0xff00) | (((val) << 8) & 0xff0000) | (((val) << 24) & 0xff000000));
	return val;
}
