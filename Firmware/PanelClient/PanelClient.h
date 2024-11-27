#pragma once

#include "UsbDevice.h"

#include "LogSession.h"
#include "DateTimeSession.h"
#include "NetworkSession.h"

class RTClock;
class ESP8266;

class PanelClient
{
public:
    PanelClient();
    
    bool init(RTClock *clock, ESP8266 *wifi);
    void process();

private:
    UsbDevice mUsb;
    LogSession mLog;
    DateTimeSession mDateTime;
    NetworkSession mNetwork;
};