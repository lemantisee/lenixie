#pragma once

#include "MessageSession.h"

class RTClock;
class DateTime;

class DateTimeSession : public MessageSession
{
public:
    DateTimeSession() = default;

    void handle(const PanelMessage &msg) override;
    void setRtc(const RTClock *clock);

private:
    SString<64> createReport(const DateTime &dateTime) const;
    const RTClock *mClock = nullptr;
};
