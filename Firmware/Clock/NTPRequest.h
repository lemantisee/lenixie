#pragma once

#include <cstdint>
#include <optional>

class Wifi;

class NTPRequest
{
public:
    NTPRequest() = default;

    void init(Wifi *wifi);
    std::optional<int64_t> getTimestamp(const char *server);

private:
    std::optional<int64_t> getNtpTimestamp();
    uint32_t htonl(uint32_t val) const;

    Wifi *mWifi = nullptr;
};