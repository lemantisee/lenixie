#pragma once

#include "UsbDevice.h"

class RTClock;
class ESP8266;
class DateTime;
class PanelMessage;

class PanelClient
{
public:
    PanelClient() = default;
    
    bool init(RTClock *clock, ESP8266 *wifi);
    void process();

private:
    void sendAck();
    void onLog();
    void sendLogEnd();
    SString<256> escapeString(const SString<128> &str) const;

    void onGetDateTime();
    void onSetDateTime(const PanelMessage &msg);
    SString<256> createDateTimeReport(const DateTime &dateTime) const;

    void onNetworkState();
    void onNetworkConnect(const PanelMessage &msg);

    void onNtpState();
    void onNtpSync();
    void onSetTimezone(const PanelMessage &msg);
    void onSetServer(const PanelMessage &msg);

    RTClock *mClock = nullptr;
    ESP8266 *mWifi = nullptr;

    UsbDevice mUsb;
};