#pragma once

#include "MessageSession.h"

class RTClock;
class DateTime;

class DateTimeSession : public MessageSession
{
public:
    DateTimeSession() = default;

    void handle(const PanelMessage &msg) override;
    void setRtc(RTClock *clock);

private:
    void onGetDateTime();
    void onSetDateTime(const SString<64> &data);

    SString<64> createReport(const DateTime &dateTime) const;
    RTClock *mClock = nullptr;
};
