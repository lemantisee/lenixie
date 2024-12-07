#include "NTPRequest.h"

#include <cstring>
#include <array>
#include <ctime>

#include "stm32f1xx.h"

#include "Wifi.h"
#include "Logger.h"
#include "SString.h"

namespace {
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

    uint32_t txTm_s = 0; // Transmit time-stamp seconds.
    uint32_t txTm_f = 0; // Transmit time-stamp fraction of a second.
};
} // namespace

void NTPRequest::init(Wifi *wifi) { mWifi = wifi; }

std::optional<int64_t> NTPRequest::getNtpTimestamp()
{
    std::array<uint8_t, 48> ntpRequset = {0x1B, 0, 0, 0, 0, 0, 0, 0, 0};
    ntp_packet ntpAnswer;

    LOG("Sending ntp request");
    mWifi->sendUDPpacket((char *)ntpRequset.data(), ntpRequset.size());

    std::array<uint8_t, 48> ntpBuffer;
    if (!mWifi->getData(ntpBuffer.data(), ntpBuffer.size())) {
        LOG("Ntp packet receive failed");
        return std::nullopt;
    }

    std::memcpy(&ntpAnswer, ntpBuffer.data(), sizeof(ntpAnswer));
    int64_t timestampSeconds = (htonl(ntpAnswer.txTm_s) - NTP_TIMESTAMP_DELTA);
    return timestampSeconds;
}

std::optional<int64_t> NTPRequest::getTimestamp(const char *server)
{
    if (!mWifi->isConnected()) {
        LOG("Not connected");
        return std::nullopt;
    }

    if (!mWifi->connectToServerUDP(server, 123)) {
        LOG("Unable to connect to ntp server");
        return std::nullopt;
    }

    for (uint8_t i = 0; i < 2; ++i) {
        if (auto timestamp = getNtpTimestamp()) {
            return timestamp;
        }
        HAL_Delay(2000);
    }

    return std::nullopt;
}

uint32_t NTPRequest::htonl(uint32_t val) const { return __builtin_bswap32(val); }
