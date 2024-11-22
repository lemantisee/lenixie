#include "DateTimeSession.h"

#include "RTClock.h"
#include "JsonObject.h"
#include "Logger.h"

void DateTimeSession::handle(const PanelMessage &msg)
{
    if (msg.cmd != GetDateTime) {
        toNext(msg);
        return;
    }

    const DateTime &dateTime = mClock->getTime();
    const SString<64> report = createReport(dateTime);

    if (report.empty()) {
        LOG("Invalid time report");
        return;
    }

    send(report);
}

void DateTimeSession::setRtc(const RTClock *clock) 
{
    mClock = clock;
}

SString<64> DateTimeSession::createReport(const DateTime &dateTime) const 
{ 
    SString<20> dateTimeStr;
    dateTimeStr.appendNumber(dateTime.year);
    dateTimeStr.appendNumber(dateTime.month);
    dateTimeStr.appendNumber(dateTime.monthDay);

    dateTimeStr.appendNumber(dateTime.hours);
    dateTimeStr.appendNumber(dateTime.minutes);
    dateTimeStr.appendNumber(dateTime.seconds);

    JsonObject json;
    json.add("id", DateTimeState);
    json.add("d", dateTimeStr.c_str());

    return json.dump();
}
