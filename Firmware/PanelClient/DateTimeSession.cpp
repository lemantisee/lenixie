#include "DateTimeSession.h"

#include "RTClock.h"
#include "JsonObject.h"
#include "Logger.h"

void DateTimeSession::handle(const PanelMessage &msg)
{
    switch (msg.cmd) {
    case GetDateTime: onGetDateTime(); break;
    case SetDateTime: onSetDateTime(msg.data); break;
    default: toNext(msg); break;
    }
}

void DateTimeSession::setRtc(RTClock *clock) 
{
    mClock = clock;
}

void DateTimeSession::onGetDateTime()
{
    const DateTime &dateTime = mClock->getTime();
    const SString<64> report = createReport(dateTime);

    if (report.empty()) {
        LOG("Invalid time report");
        return;
    }

    send(report);
}

void DateTimeSession::onSetDateTime(const SString<64> &data)
{
    DateTime dateTime;

    dateTime.year = SString<4>(data.begin(), 4).toInt();
    dateTime.month = SString<2>(data.begin() + 4, 2).toInt();
    dateTime.monthDay = SString<2>(data.begin() + 6, 2).toInt();
    dateTime.weekDay = SString<1>(data.begin() + 8, 1).toInt();

    dateTime.hours = SString<2>(data.begin() + 9, 2).toInt();
    dateTime.minutes = SString<2>(data.begin() + 11, 2).toInt();
    dateTime.seconds = SString<2>(data.begin() + 13, 2).toInt();

    mClock->setTime(dateTime);
}

SString<64> DateTimeSession::createReport(const DateTime &dateTime) const
{ 
    SString<20> dateTimeStr;
    dateTimeStr.appendNumber(dateTime.year);
    dateTimeStr.appendNumber(dateTime.month, "%02i");
    dateTimeStr.appendNumber(dateTime.monthDay, "%02i");

    dateTimeStr.appendNumber(dateTime.hours, "%02i");
    dateTimeStr.appendNumber(dateTime.minutes, "%02i");
    dateTimeStr.appendNumber(dateTime.seconds, "%02i");

    JsonObject json;
    json.add("id", DateTimeState);
    json.add("d", dateTimeStr.c_str());

    return json.dump();
}
