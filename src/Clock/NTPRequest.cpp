#include "NTPRequest.h"

#include <cstring>
#include <array>
#include <ctime>

#include "ESP8266.h"
#include "Logger.h"
#include "SString.h"

namespace
{
    constexpr uint64_t NTP_TIMESTAMP_DELTA = 2208988800;

    struct ntp_packet
    {
        uint32_t test = 0;
        uint32_t rootDelay = 0;      // Total round trip delay time.
        uint32_t rootDispersion = 0; // Max error aloud from primary clock source.
        uint32_t refId = 0;          // Reference clock ide
        uint32_t refTm_s = 0;        // Reference time-stamp seconds.
        uint32_t refTm_f = 0;        // Reference time-stamp fraction of a
        uint32_t origTm_s = 0;       // Originate time-stamp seconds.
        uint32_t origTm_f = 0;       // Originate time-stamp fraction of a
        uint32_t rxTm_s = 0;         // Received time-stamp seconds.
        uint32_t rxTm_f = 0;         // Received time-stamp fraction of a second.

        uint32_t txTm_s = 0;         // Transmit time-stamp seconds.
        uint32_t txTm_f = 0;         // Transmit time-stamp fraction of a second.
    };
}

void NTPRequest::init(ESP8266 *wifi)
{
    mWifi = wifi;
}

void NTPRequest::setTimezone(uint8_t timezone)
{
    mTimezone = timezone;
}

const NTPRequest::DateTime &NTPRequest::getTime() const
{
    return mDateTime;
}

std::optional<int64_t> NTPRequest::getNtpTimestamp()
{
    std::array<uint8_t, 48> ntpRequset = {0x1B, 0, 0, 0, 0, 0, 0, 0, 0};
    ntp_packet ntpAnswer;

    Logger::log("Sending ntp request");
    mWifi->sendUDPpacket((char *)ntpRequset.data(), ntpRequset.size());

    std::array<uint8_t, 48> ntpBuffer;
    if (!mWifi->getData(ntpBuffer.data(), ntpBuffer.size())) {
        Logger::log("Ntp packet receive failed");
        return std::nullopt;
    }

    std::memcpy(&ntpAnswer, ntpBuffer.data(), sizeof(ntpAnswer));
    int64_t timestampSeconds = (htonl(ntpAnswer.txTm_s) - NTP_TIMESTAMP_DELTA) + mTimezone * 60 * 60;
    return timestampSeconds;
}

bool NTPRequest::updateTime(int64_t timestamp)
{
    std::tm *timeVal = std::gmtime(&timestamp);
    if (!timeVal)
    {
        Logger::log("Convert timestamp failed");
        return false;
    }

    mDateTime.monthDay = timeVal->tm_mday;
    mDateTime.weekDay = timeVal->tm_wday;
    mDateTime.month = timeVal->tm_mon;
    mDateTime.year = timeVal->tm_year + 1900;
    mDateTime.hours = timeVal->tm_hour;
    mDateTime.minutes = timeVal->tm_min;
    mDateTime.seconds = timeVal->tm_sec;

    if(mDateTime.weekDay == 0) {
        mDateTime.weekDay = 7;
    }

    SString<100> str;
    str.append("NTP time: ").appendNumber(mDateTime.monthDay).append("-").appendNumber(mDateTime.month).append("/").appendNumber(mDateTime.weekDay).append(" ");
    str.appendNumber(mDateTime.hours).append(":").appendNumber(mDateTime.minutes).append(":").appendNumber(mDateTime.seconds);
    Logger::log(str.c_str());
    return true;
}

bool NTPRequest::request(const char *server)
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
        if (auto timestamp = getNtpTimestamp()) {
            if (updateTime(*timestamp)) {
                return true;
            }
        }
        HAL_Delay(2000);
    }

    return false;
}

uint32_t NTPRequest::htonl(uint32_t val) const
{
    return __builtin_bswap32(val);
}
