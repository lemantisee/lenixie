#pragma once

#include "MessageSession.h"

class RTClock;

class NtpSession : public MessageSession
{
public:
    void handle(const PanelMessage &msg) override;
    void setClock(RTClock *clock);

private:
    void onState();
    void onSync();
    void onSetTimezone(const PanelMessage &msg);
    void onSetServer(const PanelMessage &msg);
    RTClock *mClock = nullptr;
};