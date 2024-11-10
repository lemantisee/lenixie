#pragma once

#include "SString.h"

class UsbDevice;

class LogDump
{
public:
    void dump(UsbDevice &usbDevice);

private:
    SString<64> createLogUnit(const SString<48> &str, bool end) const;
    std::array<SString<48>, 6> splitString(const SString<256> &str) const;
    SString<256> escapeString(const SString<128> &str) const;
};
