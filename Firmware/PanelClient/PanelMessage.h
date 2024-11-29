#pragma once

#include "SString.h"
#include "JsonObject.h"

class PanelMessage
{
public:
    enum PanelCommandId {
        UnknownCommand = 0,
        GetDateTime = 1,
        DateTimeState = 2,
        GetLog = 3,
        LogUnit = 4,
        LogEnd = 6,
        SetDateTime = 7,
        GetNetworkState = 8,
        NetworkState = 9,
        ConnectToWifi = 10,
        SetTimezone = 11,
        SetNtpServer = 12,
        SyncNtpTime = 13,
        GetNtpState = 14,
        NtpState = 15,
        
        MessageAck = 50,
    };

    PanelMessage() = default;

    PanelMessage(PanelCommandId cmd) { mJson.add("id", cmd); }

    static PanelMessage fromReport(const SString<256> &report)
    {
        PanelMessage msg;
        msg.mJson = JsonObject(report.c_str());
        return msg;
    }

    PanelCommandId getCmd() const { return PanelCommandId(mJson.getInt("id", UnknownCommand)); }
    int getInt(const char *key, int defaultValue) const { return mJson.getInt(key, defaultValue); }
    SString<256> getString(const char *key) const { return mJson.get(key); }

    void set(const char *key, int value) { mJson.add(key, value); }
    void add(const char *key, const char *str) { mJson.add(key, str); }
    const SString<256> &toString() { return mJson.dump(); }

private:
    JsonObject mJson;
};