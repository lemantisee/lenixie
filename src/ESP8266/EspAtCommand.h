#pragma once

#include "SString.h"

class EspAtCommand
{
public:
    EspAtCommand() = default;
    EspAtCommand(const char *str);

    EspAtCommand &add(const char *str);
    EspAtCommand &add(uint32_t number);
    const char *string() const;
private:

    SString<255> mString;
    bool mHasArgument = false;
};
