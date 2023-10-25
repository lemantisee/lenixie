#include "EspAtCommand.h"

EspAtCommand::EspAtCommand(const char *str)
{
    mString.append(str);
}

EspAtCommand &EspAtCommand::add(const char *str)
{
    if (mHasArgument) {
        mString.append(",");
    }
    mString.append("\"").append(str).append("\"");
    mHasArgument = true;

    return *this;
}

EspAtCommand &EspAtCommand::add(uint32_t number)
{
    if (mHasArgument) {
        mString.append(",");
    }
    mString.appendNumber(number);
    mHasArgument = true;

    return *this;
}

const char *EspAtCommand::string() const
{
    return mString.c_str();
}
