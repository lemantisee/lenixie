#pragma once

#include "PanelCommand.h"
#include "SString.h"
#include "JsonObject.h"

class PanelMessage
{
public:
    static PanelMessage fromReport(const SString<256> &report)
    {
        PanelMessage msg;
        msg.mJson = JsonObject(report.c_str());
        return msg;
    }

    PanelCommandId getCmd() const { return PanelCommandId(mJson.getInt("id", UnknownCommand)); }
    int getInt(const char *key, int defaultValue) const { return mJson.getInt(key, defaultValue); }
    SString<256> getString(const char *key) const { return mJson.get(key); }

private:
    PanelCommandId mCmd = UnknownCommand;
    JsonObject mJson;
};