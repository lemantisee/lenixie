#pragma once

#include "PanelMessage.h"
#include "UsbDevice.h"

class MessageSession
{
public:
    virtual void handle(const PanelMessage &msg) = 0;

    void setNext(MessageSession *session) { mNext = session; }
    void setClient(UsbDevice *usb) { mUsb = usb; }

protected:
    void send(const SString<256> &report) { mUsb->sendData(report); }
    void toNext(const PanelMessage &msg)
    {
        if (mNext) {
            mNext->handle(msg);
        }
    }

private:
    MessageSession *mNext = nullptr;
    UsbDevice *mUsb = nullptr;
};
