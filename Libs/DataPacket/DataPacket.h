#pragma once

#include <cstdint>

#include <SString.h>

class DataPacket
{
public:
    enum PacketType {
        UnknownPacket = 0,
        PacketPayload = 1,
        EndPacket = 2,
        PacketAck = 3,
    };

    DataPacket() = default;
    DataPacket(const uint8_t *data, uint32_t size);
    DataPacket(const SString<60> &payload);

    SString<60> getPayload() const;
    PacketType getType() const;
    void setEnd();

    const uint8_t *data() const;
    uint8_t size() const;

    static const uint8_t payloadMaxSize = 60;
    static std::array<DataPacket, 6> splitToPackets(const SString<256> &data);

private:
    std::array<uint8_t, 64> mData = {};
};