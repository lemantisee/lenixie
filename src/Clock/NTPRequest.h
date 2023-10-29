#pragma once

#include <cstdint>
#include <optional>

class ESP8266;

class NTPRequest
{
public:
    struct DateTime {
        uint32_t year = 0;
        uint8_t month = 0;
        uint8_t monthDay = 0;
        uint8_t weekDay = 0;
        uint8_t hours = 0;
        uint8_t minutes = 0;
        uint8_t seconds = 0;
    };

    NTPRequest () = default;

    void init(ESP8266 *wifi);
    void setTimezone(uint8_t timezone);

    const DateTime &getTime() const;
    bool request(const char *server);

private:
    std::optional<int64_t> getNtpTimestamp();
    bool updateTime(int64_t timestamp);
    uint32_t htonl(uint32_t val) const;

    ESP8266 *mWifi = nullptr;
    DateTime mDateTime;
    uint8_t mTimezone = 0;
};