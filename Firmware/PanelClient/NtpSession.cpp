#include "NtpSession.h"

#include "RTClock.h"

#include "Logger.h"

void NtpSession::handle(const PanelMessage &msg)
{
    switch (msg.getCmd()) {
    case PanelMessage::GetNtpState: onState(); break;
    case PanelMessage::SyncNtpTime: onSync(); break;
    case PanelMessage::SetNtpServer: onSetServer(msg); break;
    case PanelMessage::SetTimezone: onSetTimezone(msg); break;
    default: toNext(msg); break;
    }
}

void NtpSession::setClock(RTClock *clock) { mClock = clock; }

void NtpSession::onState() 
{
    JsonObject json;
    json.add("id", PanelMessage::NtpState);
    json.add("url", mClock->getNtpServer().c_str());
    json.add("t", mClock->getTimeZone());

    send(json.dump());
}

void NtpSession::onSync() { mClock->syncNtp(); }

void NtpSession::onSetTimezone(const PanelMessage &msg) 
{
    int timezone = msg.getInt("t", 15);

    if (timezone > 12 || timezone < -12) {
        LOG_ERROR("Time zone value out of range: %i", timezone);
        return;
    }

    LOG("Setting time zone: %i", timezone);

    mClock->setTimeZone(int8_t(timezone));
}

void NtpSession::onSetServer(const PanelMessage &msg)
{
    SString<256> url = msg.getString("url");
    if (url.empty()) {
        LOG_ERROR("Empty ntp url");
        return;
    }

    LOG("Setting ntp url: %s", url.c_str());

    mClock->setNtpServer(SString<128>(url.data(), url.size()));
}
