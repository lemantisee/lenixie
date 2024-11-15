#include "DateTime.h"

#include <ctime>

#include "Logger.h"

bool DateTime::isNull() const { return year == 0; }

DateTime DateTime::fromTimestamp(int64_t timestamp)
{
    std::tm *timeVal = std::gmtime(&timestamp);
    if (!timeVal) {
        LOG("Convert timestamp failed");
        return {};
    }

    DateTime dateTime;

    dateTime.monthDay = timeVal->tm_mday;
    dateTime.weekDay = timeVal->tm_wday;
    dateTime.month = timeVal->tm_mon;
    dateTime.year = timeVal->tm_year + 1900;
    dateTime.hours = timeVal->tm_hour;
    dateTime.minutes = timeVal->tm_min;
    dateTime.seconds = timeVal->tm_sec;

    if (dateTime.weekDay == 0) {
        dateTime.weekDay = 7;
    }

    return dateTime;
}

DateTime DateTime::localDatetime(int64_t utcTimestamp, uint8_t timezone)
{
    int64_t localTimestamp = utcTimestamp + timezone * 60 * 60;

    std::tm *timeVal = std::gmtime(&localTimestamp);
    if (!timeVal) {
        LOG("Convert timestamp failed");
        return {};
    }

    int8_t dst = calculateDST(timeVal->tm_mon, timeVal->tm_mday, timeVal->tm_wday,
                              timeVal->tm_hour);

    localTimestamp += dst * 60 * 60;

    return fromTimestamp(localTimestamp);
}

int8_t DateTime::calculateDST(uint8_t month_, uint8_t monthday_, uint8_t weekday_, uint8_t hours_)
{
    if (weekday_ == 0) {
        weekday_ = 7;
    }

    // For Ukraine
    // set back 1 hour on the last week of October on 3am
    // set forward 1 hour on the last week of March on 4am

    if (month_ > 9 || month_ < 2) {
        return -1;
    }

    if (month_ > 2 && month_ < 8) {
        return 0;
    }

    const uint8_t lastSunday = getLastSunday(monthday_, weekday_);
    const bool edge = monthday_ > lastSunday || (monthday_ == lastSunday && hours_ >= 3);

    if (month_ == 9) {
        return edge ? -1 : 0;
    }

    if (month_ == 2) {
        return edge ? 0 : -1;
    }

    return 0;
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

uint8_t DateTime::getLastSunday(uint8_t monthday, uint8_t weekday)
{
    // March and October has 31 days
    // we interested in period from 25 day
    // only this days can be in the end of last week in month which has 31 day

    if (monthday < 25) {
        return monthday;
    }

    uint8_t forwardDay = monthday + 7 - weekday;
    if (forwardDay <= 31) {
        return forwardDay;
    }

    return monthday - weekday;
}