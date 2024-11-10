#include "DateTime.h"

void DateTime::addDstHour(int8_t dstHour)
{
    hours += dstHour;
    int8_t m = month;
    int8_t md = monthDay;
    int8_t wd = weekDay;

    if (int8_t(hours) < 0) {
        hours = 24 - std::abs(int8_t(hours));

        --monthDay;
        if (monthDay == 0) {
            --month;
            if (int8_t(month) < 0) {
                month = 11;
                --year;
            }

            monthDay = daysInMonth(year, month);
        }

        --weekDay;
        if (weekDay == 0) {
            weekDay = 7;
        }
    }
}

uint8_t DateTime::daysInMonth(uint32_t year, uint8_t month) const
{
    const bool leapYear = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);

    switch (month + 1) {
    case 2: return leapYear ? 29 : 28;
    case 1:
    case 3:
    case 5:
    case 7:
    case 8:
    case 10:
    case 12: return 31;
    default: break;
    }

    return 30;
}