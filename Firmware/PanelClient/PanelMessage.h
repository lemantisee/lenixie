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
        LogEnd = 5,
        SetDateTime = 6,
        GetNetworkState = 7,
        NetworkState = 8,
        ConnectToWifi = 9,
        ConnectToLastWifi = 10,
        DisconnectWifi = 11,
        SetTimezone = 12,
        SetNtpServer = 13,
        SyncNtpTime = 14,
        GetNtpState = 15,
        NtpState = 16,
        GetVersion = 17,
        VersionInfo = 18,

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