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

    bool isNull() const;
    static DateTime fromTimestamp(int64_t timestamp);
    static DateTime localDatetime(int64_t utcTimestamp, uint8_t timezone);

private:
    static int8_t calculateDST(uint8_t month_, uint8_t monthday_, uint8_t weekday_, uint8_t hours_);
    static uint8_t getLastSunday(uint8_t monthday, uint8_t weekday);

    uint8_t daysInMonth(uint32_t year, uint8_t month) const;
};
