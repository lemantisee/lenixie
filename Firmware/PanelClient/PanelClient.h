#pragma once

#include "UsbDevice.h"

#include "LogSession.h"
#include "DateTimeSession.h"

class RTClock;

class PanelClient
{
public:
    PanelClient();
    
    bool init(RTClock *clock);
    void process();

private:
    UsbDevice mUsb;
    LogSession mLog;
    DateTimeSession mDateTime;
};