#pragma once

#include <cstdint>
#include <cstdlib>

class DateTime
{
    
public:
    uint32_t year = 0;
    uint8_t month = 0;
    uint8_t monthDay = 0;
    uint8_t weekDay = 0;
    uint8_t hours = 0;
    uint8_t minutes = 0;
    uint8_t seconds = 0;

    void addDstHour(int8_t dstHour);

private:
    uint8_t daysInMonth(uint32_t year, uint8_t month) const;
};
