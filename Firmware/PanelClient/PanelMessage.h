#pragma once

#include "PanelCommand.h"
#include "SString.h"
#include "JsonObject.h"

class PanelMessage
{
public:
    PanelCommandId cmd = UnknownCommand;
    SString<256> data;

    static PanelMessage fromReport(const SString<256> &report)
    {
        JsonObject inMessage(report.c_str());

        PanelMessage msg;
        msg.cmd = PanelCommandId(inMessage.getInt("id", UnknownCommand));
        msg.data = inMessage.get("d");

        return msg;
    }
};