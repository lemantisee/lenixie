#pragma once

#include "PanelMessage.h"
#include "UsbDevice.h"

class MessageSession
{
public:
    virtual void handle(const PanelMessage &msg) = 0;

    void setNext(MessageSession *session) { mNext = session; }
    MessageSession *getNext() const { return mNext; }
    void setClient(UsbDevice *usb) { mUsb = usb; }

protected:
    void send(const SString<64> &report) { mUsb->sendData(report); }

private:
    MessageSession *mNext = nullptr;
    UsbDevice *mUsb = nullptr;
};
