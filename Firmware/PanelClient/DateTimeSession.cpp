#include "DateTimeSession.h"

#include "RTClock.h"
#include "JsonObject.h"
#include "Logger.h"

void DateTimeSession::handle(const PanelMessage &msg)
{
    switch (msg.getCmd()) {
    case PanelMessage::GetDateTime: onGetDateTime(); break;
    case PanelMessage::SetDateTime: onSetDateTime(msg); break;
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
    const SString<256> report = createReport(dateTime);

    if (report.empty()) {
        LOG("Invalid time report");
        return;
    }

    send(report);
}

void DateTimeSession::onSetDateTime(const PanelMessage &msg)
{
    DateTime dateTime;

    dateTime.year = msg.getInt("y", 0);
    dateTime.month = msg.getInt("mn", 0);
    dateTime.monthDay = msg.getInt("md", 0);
    dateTime.weekDay = msg.getInt("w", 0);

    dateTime.hours = msg.getInt("h", 0);
    dateTime.minutes = msg.getInt("m", 0);
    dateTime.seconds = msg.getInt("s", 0);

    mClock->setTime(dateTime);
}

SString<256> DateTimeSession::createReport(const DateTime &dateTime) const
{ 
    JsonObject json;
    json.add("id", PanelMessage::DateTimeState);
    json.add("y", dateTime.year);
    json.add("mn", dateTime.month);
    json.add("md", dateTime.monthDay);

    json.add("h", dateTime.hours);
    json.add("m", dateTime.minutes);
    json.add("s", dateTime.seconds);

    return json.dump();
}
