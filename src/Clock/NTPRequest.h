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
    std::optional<int64_t> getTimestamp(const char *server);

private:
    std::optional<int64_t> getNtpTimestamp();
    uint32_t htonl(uint32_t val) const;

    ESP8266 *mWifi = nullptr;
};