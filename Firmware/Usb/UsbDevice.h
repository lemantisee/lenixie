#pragma once

#include "UsbCustomHid.h"

#include "UsbDriverF103.h"
#include "UsbDeviceDescriptor.h"
#include "SString.h"
#include "DataPacket.h"

class UsbDevice : public UsbCustomHid
{
public:
    bool init();
    void sendData(const SString<256> &data);
    SString<256> popData();
    void process();

protected:
    void onReceive(uint8_t *state, uint32_t size) override;
    uint8_t *getReportDescriptor() const override;

private:
    UsbDriverF103 mDriver;
    UsbHandle mHandle;
    UsbDeviceDescriptor mDescriptor;

    SString<256> mOutcomeBuffer;
    SString<256> mIncomeBuffer;
    volatile bool mMsgReceived = false;
    volatile bool mHostReceived = false;
};
