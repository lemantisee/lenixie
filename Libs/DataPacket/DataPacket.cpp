#include "DataPacket.h"

namespace {

//2 bytes unique header
//1 byte type
//1 byte size
//60 bytes payload

const uint16_t header = 0x1A56;
const uint8_t headerIndex = 0;
const uint8_t payloadMaxSize = 60;
const uint8_t typeIndex = 2;
const uint8_t sizeIndex = 3;
const uint8_t payloadIndex = 4;
} // namespace

DataPacket::DataPacket(const uint8_t *data, uint32_t size)
{
    if (size != 64) {
        return;
    }

    uint16_t packetHeader = 0;
    std::memcpy(&packetHeader, data, 2);

    if (packetHeader != header) {
        return;
    }

    PacketType type = PacketType(data[typeIndex]);

    const uint8_t payloadSize = data[sizeIndex];
    // if ((payloadSize == 0 && type != PacketAck) || payloadSize > 60) {
    //     return;
    // }
    
    std::memcpy(mData.data(), data, 2);
    mData[typeIndex] = type;
    mData[sizeIndex] = payloadSize;
    if (payloadSize > 0) {
        std::memcpy(&mData[payloadIndex], &data[payloadIndex], payloadSize);
    }
}

DataPacket::DataPacket(const SString<60> &payload)
{
    if (payload.size() > payloadMaxSize) {
        return;
    }

    std::memcpy(mData.data() + headerIndex, &header, 2);
    mData[typeIndex] = PacketPayload;
    mData[sizeIndex] = uint8_t(payload.size());
    std::memcpy(&mData[payloadIndex], payload.c_str(), payload.size());
}

SString<60> DataPacket::getPayload() const
{
    const size_t layloadSize = mData[sizeIndex];
    return SString<60>((const char *)(&mData[payloadIndex]), layloadSize);
}

DataPacket::PacketType DataPacket::getType() const { return PacketType(mData[typeIndex]); }

void DataPacket::setEnd() 
{
    mData[typeIndex] = EndPacket;
}

const uint8_t *DataPacket::data() const { return mData.data(); }

uint8_t DataPacket::size() const { return mData.size(); }

std::array<DataPacket, 6> DataPacket::splitToPackets(const SString<256> &data)
{
    std::array<DataPacket, 6> packets;

    size_t strSize = data.size();
    const char *ptr = data.c_str();

    for (DataPacket &packet : packets) {
        const size_t sizeToCopy = std::min<size_t>(strSize, DataPacket::payloadMaxSize);

        packet = DataPacket(SString<60>(ptr, sizeToCopy));
        strSize -= sizeToCopy;
        ptr += sizeToCopy;

        if (strSize == 0) {
            packet.setEnd();
            break;
        }
    }

    return packets;
}
