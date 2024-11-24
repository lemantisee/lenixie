#include <catch2/catch_test_macros.hpp>

#include "DataPacket.h"

#include <string>

TEST_CASE("DataPacket: capacity")
{
    const uint16_t packetHeaderId = 0x1A56;

    SString<256> str = "Yourself off its pleasant ecstatic now law.Ye their mirth seems of songs. "
                       "Prospect out bed contempt separate.";

    const std::string packetPayload0 = "Yourself off its pleasant ecstatic now law.Ye their mirth se";
    const std::string packetPayload1 = "ems of songs. Prospect out bed contempt separate.";

    std::array<DataPacket, 6> packets = DataPacket::splitToPackets(str);

    std::array<uint8_t, 64> packetData0 = {};
    std::memcpy(packetData0.data(), &packetHeaderId, 2);
    packetData0[2] = DataPacket::PacketPayload;
    packetData0[3] = packetPayload0.size();
    std::memcpy(&packetData0[4], packetPayload0.data(), packetPayload0.size());

    REQUIRE(packets[0].getType() == DataPacket::PacketPayload);
    REQUIRE(packets[0].getPayload() == packetPayload0.c_str());
    REQUIRE(packets[0].size() == packetData0.size());
    REQUIRE(std::memcmp(packets[0].data(), packetData0.data(), packetData0.size()) == 0);

    std::array<uint8_t, 64> packetData1 = {};
    std::memcpy(packetData1.data(), &packetHeaderId, 2);
    packetData1[2] = DataPacket::EndPacket;
    packetData1[3] = packetPayload1.size();
    std::memcpy(&packetData1[4], packetPayload1.data(), packetPayload1.size());

    REQUIRE(packets[1].getType() == DataPacket::EndPacket);
    REQUIRE(packets[1].getPayload() == packetPayload1.c_str());
    REQUIRE(packets[1].size() == packetData1.size());
    REQUIRE(std::memcmp(packets[1].data(), packetData1.data(), packetData1.size()) == 0);

    REQUIRE(packets[2].getType() == DataPacket::UnknownPacket);
    REQUIRE(packets[3].getType() == DataPacket::UnknownPacket);
    REQUIRE(packets[4].getType() == DataPacket::UnknownPacket);
    REQUIRE(packets[5].getType() == DataPacket::UnknownPacket);
}