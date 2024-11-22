#pragma once

#include "PanelCommand.h"
#include "SString.h"
#include "JsonObject.h"

class PanelMessage
{
public:
    MonitorCommandId cmd = UnknownCommand;
    SString<64> data;

    static PanelMessage fromReport(const SString<64> &report)
    {
        JsonObject inMessage(report);

        PanelMessage msg;
        msg.cmd = MonitorCommandId(inMessage.getInt("id", UnknownCommand));
        msg.data = inMessage.get("d");

        return msg;
    }
};