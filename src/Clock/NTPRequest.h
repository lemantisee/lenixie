#pragma once

#include <cstdint>
#include <optional>

#include "DateTime.h"

class ESP8266;

class NTPRequest
{
public:
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