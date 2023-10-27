#include "NTPHandle.h"

#include <cstring>
#include <array>
#include <ctime>

#include "ESP8266.h"
#include "Logger.h"
#include "SString.h"

namespace
{
    constexpr uint64_t NTP_TIMESTAMP_DELTA = 2208988800;
}

void NTPHandle::init(ESP8266 *wifi)
{
    mWifi = wifi;
}

bool NTPHandle::getNtpRequest()
{
    std::array<uint8_t, 48> ntpRequset = {0x1B, 0, 0, 0, 0, 0, 0, 0, 0};
    mNtpAnswer = {};

    Logger::log("Sending ntp request");
    mWifi->sendUDPpacket((char *)ntpRequset.data(), ntpRequset.size());

    std::array<uint8_t, 48> ntpBuffer;
    if (!mWifi->getData(ntpBuffer.data(), ntpBuffer.size())) {
        Logger::log("Ntp packet receive failed");
        return false;
    }

    std::memcpy(&mNtpAnswer, ntpBuffer.data(), sizeof(mNtpAnswer));
    mTimestampMs = (htonl(mNtpAnswer.txTm_s) - NTP_TIMESTAMP_DELTA);
    return true;
}

bool NTPHandle::updateTime()
{
    std::time_t timestamp = mTimestampMs;
    std::tm *timeVal = std::gmtime(&timestamp);
    if (!timeVal)
    {
        Logger::log("Convert timestamp failed");
        return false;
    }

    mHours = timeVal->tm_hour;
    mMinutes = timeVal->tm_min;
    mSeconds = timeVal->tm_sec;

    SString<100> str;
    str.append("NTP time: ").appendNumber(mHours).append(":").appendNumber(mMinutes).append(":").appendNumber(mSeconds);
    Logger::log(str.c_str());
    return true;
}

uint8_t NTPHandle::getHours()
{
    return mHours;
}

uint8_t NTPHandle::getMinutes()
{
    return mMinutes;
}

uint8_t NTPHandle::getSeconds()
{
    return mSeconds;
}

bool NTPHandle::process(const char *server)
{
    if (!mWifi->isConnected())
    {
        return false;
    }

    if (!mWifi->connectToServerUDP(server, 123))
    {
        return false;
    }

    for (uint8_t i = 0; i < 2; ++i) {
        if (getNtpRequest()) {
            if (updateTime()) {
                return true;
            }
        }
        HAL_Delay(2000);
    }

    return false;
}

uint32_t NTPHandle::htonl(uint32_t val) const
{
    return __builtin_bswap32(val);
}
