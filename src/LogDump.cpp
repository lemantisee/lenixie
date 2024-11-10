#include "LogDump.h"

#include "UsbDevice.h"
#include "JsonObject.h"
#include "MonitorCommand.h"

void LogDump::dump(UsbDevice &usbDevice)
{
    while (true) {
        if (Logger::empty()) {
            JsonObject j;
            j.add("id", LogEnd);
            usbDevice.sendData(j.dump());
            break;
        }

        SString<128> str = Logger::pop();
        if (str.empty()) {
            break;
        }

        SString<256> escapedString = escapeString(str);

        for (const SString<48> &token : splitString(escapedString)) {
            if (token.empty()) {
                continue;
            }

            const bool end = token.size() < token.capacity();

            SString<64> msg = createLogUnit(token, end);
            usbDevice.sendData(msg);

            SString<64> report;
            while (report.empty()) {
                report = usbDevice.popData();
            }

            if (JsonObject(report).getInt("id", UnknownCommand) != GetLog) {
                return;
            }
        }
    }
}

SString<64> LogDump::createLogUnit(const SString<48> &str, bool end) const
{
    JsonObject j;

    j.add("id", end ? LogUnitEnd : LogUnit);
    j.add("d", str.c_str());
    return j.dump();
}

std::array<SString<48>, 6> LogDump::splitString(const SString<256> &str) const
{
    const size_t tokenCapacity = 48;
    size_t strSize = str.size();
    std::array<SString<48>, 6> strings;

    const char *ptr = str.c_str();
    for (SString<48> &token : strings) {
        if (strSize == 0) {
            break;
        }

        const size_t sizeToCopy = std::min(strSize, tokenCapacity);

        token = SString<48>(ptr, sizeToCopy);
        strSize -= sizeToCopy;
        ptr += sizeToCopy;
    }

    return strings;
}

SString<256> LogDump::escapeString(const SString<128> &str) const
{
    SString<256> escString;

    for (char c: str) {
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