#pragma once

#include <cstdint>

class ESP8266;

class NTPHandle
{
public:
    NTPHandle () = default;

    void init(ESP8266 *wifi);
    uint8_t getHours();
    uint8_t getMinutes();
    uint8_t getSeconds();

    bool process(const char *server);

private:
    bool getNtpRequest();
    bool updateTime();
    uint32_t htonl(uint32_t val) const;
    void delay(uint32_t ticks) const;

    struct ntp_packet
    {
        uint32_t test = 0;
        uint32_t rootDelay = 0;      // 32 bits. Total round trip delay time.
        uint32_t rootDispersion = 0; // 32 bits. Max error aloud from primary clock source.
        uint32_t refId = 0;          // 32 bits. Reference clock identifier.

        uint32_t refTm_s = 0;        // 32 bits. Reference time-stamp seconds.
        uint32_t refTm_f = 0;        // 32 bits. Reference time-stamp fraction of a second.

        uint32_t origTm_s = 0;       // 32 bits. Originate time-stamp seconds.
        uint32_t origTm_f = 0;       // 32 bits. Originate time-stamp fraction of a second.

        uint32_t rxTm_s = 0;         // 32 bits. Received time-stamp seconds.
        uint32_t rxTm_f = 0;         // 32 bits. Received time-stamp fraction of a second.

        uint32_t txTm_s = 0;         // 32 bits and the most important field the client cares about. Transmit time-stamp seconds.
        uint32_t txTm_f = 0;         // 32 bits. Transmit time-stamp fraction of a second.

    };

    ESP8266 *mWifi = nullptr;
    char *mHost = nullptr;
    uint16_t mPort = 0;
 
    ntp_packet mNtpAnswer;
    uint8_t mHours = 0;
    uint8_t mMinutes = 0;
    uint8_t mSeconds = 0;
    uint8_t mTimeZone = 0;
    uint32_t mSecondsFromStart = 0;
    int64_t mTimestampMs = 0;
};