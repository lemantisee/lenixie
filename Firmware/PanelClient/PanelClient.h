#pragma once

#include "UsbDevice.h"

class RTClock;
class Wifi;
class DateTime;
class PanelMessage;

class PanelClient
{
public:
    PanelClient() = default;
    
    bool init(RTClock *clock, Wifi *wifi);
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
    void onNetworkLastConnect();
    void onNetworkDisconnect();

    void onNtpState();
    void onNtpSync();
    void onSetTimezone(const PanelMessage &msg);
    void onSetServer(const PanelMessage &msg);

    RTClock *mClock = nullptr;
    Wifi *mWifi = nullptr;

    UsbDevice mUsb;
};