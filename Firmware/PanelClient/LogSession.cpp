#include "LogSession.h"

#include "Logger.h"

void LogSession::handle(const PanelMessage &msg)
{
    if (msg.getCmd() != PanelMessage::GetLog) {
        toNext(msg);
        return;
    }

    if (Logger::empty()) {
        sendEnd();
        return;
    }

    SString<128> log = Logger::pop();
    if (log.empty()) {
        sendEnd();
        return;
    }

    SString<256> escapedString = escapeString(log);

    JsonObject j;

    j.add("id", PanelMessage::LogUnit);
    j.add("d", escapedString.c_str());

    send(j.dump());
}

void LogSession::sendEnd()
{
    JsonObject j;
    j.add("id", PanelMessage::LogEnd);
    send(j.dump());
}

SString<256> LogSession::escapeString(const SString<128> &str) const
{
    SString<256> escString;

    for (char c : str) {
        if (c == 0) {
            break;
        }

        if (c == '"') {
            escString += '\\';
        }

        if (c > 31 && c < 127) {
            escString += c;
            continue;
        }

        const SString<256> hex = Logger::format("<0x%02X>", c);
        escString.append(hex.c_str());
    }

    return escString;
}