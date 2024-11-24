#include "UsbDevice.h"

#include "UsbDriver.h"
#include "DataAlign.h"

namespace {
const uint8_t deviceId = 0;
__ALIGN_BEGIN static uint8_t reportDescriptor[customHidReportDescriptoSize] __ALIGN_END = {
    0x06, 0x00, 0xff, // Usage Page(Undefined )
    0x09, 0x01,       // USAGE (Undefined)
    0xa1, 0x01,       // COLLECTION (Application)
    0x15, 0x00,       // LOGICAL_MINIMUM (0)
    0x26, 0xff, 0x00, // LOGICAL_MAXIMUM (255)
    0x75, 0x08,       // REPORT_SIZE (8)
    0x95, 0x40,       // REPORT_COUNT (64)
    0x09, 0x01,       // USAGE (Undefined)
    0x81, 0x02,       // INPUT (Data,Var,Abs)
    0x95, 0x40,       // REPORT_COUNT (64)
    0x09, 0x01,       // USAGE (Undefined)
    0x91, 0x02,       // OUTPUT (Data,Var,Abs)
    0x95, 0x40,       // REPORT_COUNT (64)
    0x09, 0x01,       // USAGE (Undefined)
    0xb1, 0x02,       // FEATURE (Data,Var,Abs)
    0xC0              /* END_COLLECTION */
};
} // namespace

bool UsbDevice::init()
{
    if (!mHandle.init(&mDescriptor, deviceId, &mDriver)) {
        return false;
    }

    mHandle.registerClass(this);

    return mHandle.start();
}

void UsbDevice::sendData(const SString<256> &data) { mOutcomeBuffer = data; }

SString<256> UsbDevice::popData()
{
    if (mMsgReceived) {
        mMsgReceived = false;
        SString<256> tmp = mIncomeBuffer;
        mIncomeBuffer.clear();
        return tmp;
    }

    return {};
}

void UsbDevice::process()
{
    if (mOutcomeBuffer.empty()) {
        return;
    }

    for (const DataPacket &packet : DataPacket::splitToPackets(mOutcomeBuffer)) {
        if (packet.getType() == DataPacket::UnknownPacket) {
            continue;
        }

        mHostReceived = false;
        while (!mHostReceived) {
        }

        sendReport(&mHandle, {packet.data(), packet.size()});

        if (packet.getType() == DataPacket::EndPacket) {
            break;
        }
    }

    mOutcomeBuffer.clear();
}

void UsbDevice::onReceive(uint8_t *state, uint32_t size)
{
    DataPacket packet(state, size);

    switch (packet.getType()) {
    case DataPacket::PacketPayload: {
        const SString<60> &payload = packet.getPayload();
        mIncomeBuffer.append(payload.c_str(), payload.size());
    } break;
    case DataPacket::EndPacket: {
        const SString<60> &payload = packet.getPayload();
        mIncomeBuffer.append(payload.c_str(), payload.size());
        mMsgReceived = true;
    } break;
    case DataPacket::PacketAck: mHostReceived = true; break;
    default: break;
    }
}

uint8_t *UsbDevice::getReportDescriptor() const { return reportDescriptor; }