#include "PanelClient.h"

#include "RTClock.h"
#include "Wifi.h"
#include "PanelMessage.h"
#include "Version.h"
#include "Settings.h"

#include "Logger.h"

class SendAckGuard
{
public:
    SendAckGuard(UsbDevice &usb) : mUsb(usb) {}
    ~SendAckGuard()
    {
        PanelMessage ack(PanelMessage::MessageAck);
        mUsb.sendData(ack.toString());
    }

private:
    UsbDevice &mUsb;
};

bool PanelClient::init(RTClock *clock, Wifi *wifi)
{
    mClock = clock;
    mWifi = wifi;

    return mUsb.init();
}

void PanelClient::process()
{
    mUsb.process();

    const SString<256> inBuffer = mUsb.popData();
    if (inBuffer.empty()) {
        return;
    }

    const PanelMessage msg = PanelMessage::fromReport(inBuffer);

    switch (msg.getCmd()) {
    case PanelMessage::GetLog: onLog(); break;

    case PanelMessage::GetDateTime: onGetDateTime(); break;
    case PanelMessage::SetDateTime: onSetDateTime(msg); break;

    case PanelMessage::GetNetworkState: onNetworkState(); break;
    case PanelMessage::ConnectToWifi: onNetworkConnect(msg); break;
    case PanelMessage::ConnectToLastWifi: onNetworkLastConnect(); break;
    case PanelMessage::DisconnectWifi: onNetworkDisconnect(); break;

    case PanelMessage::GetNtpState: onNtpState(); break;
    case PanelMessage::SyncNtpTime: onNtpSync(); break;
    case PanelMessage::SetNtpServer: onSetServer(msg); break;
    case PanelMessage::SetTimezone: onSetTimezone(msg); break;

    case PanelMessage::GetVersion: onVersion(); break;

    case PanelMessage::GetDndState: onDndState(); break;
    case PanelMessage::SetDndHours: onSetDnd(msg); break;
    case PanelMessage::EnableDnd: onEnableDnd(msg); break;

    default:
        LOG_ERROR("Unknown message: %i", msg.getCmd());
        sendAck();
        break;
    }
}

void PanelClient::sendAck() 
{
    PanelMessage ack(PanelMessage::MessageAck);
    mUsb.sendData(ack.toString());
}

void PanelClient::onLog()
{
    if (Logger::empty()) {
        sendLogEnd();
        return;
    }

    SString<128> log = Logger::pop();
    if (log.empty()) {
        sendLogEnd();
        return;
    }

    SString<256> escapedString = escapeString(log);

    JsonObject j;

    j.add("id", PanelMessage::LogUnit);
    j.add("d", escapedString.c_str());

    mUsb.sendData(j.dump());
}

void PanelClient::sendLogEnd()
{
    JsonObject j;
    j.add("id", PanelMessage::LogEnd);
    mUsb.sendData(j.dump());
}

SString<256> PanelClient::escapeString(const SString<128> &str) const
{
    SString<256> escString;

    for (char c : str) {
        if (c == 0) {
            break;
        }

        if (c == '"') {
            escString += '\\';
        }

        if (c > 31 && c < 127) {
            escString += c;
            continue;
        }

        const SString<256> hex = Logger::format("<0x%02X>", c);
        escString.append(hex.c_str());
    }

    return escString;
}

void PanelClient::onGetDateTime()
{
    const DateTime &dateTime = mClock->getTime();
    const SString<256> report = createDateTimeReport(dateTime);

    if (report.empty()) {
        LOG("Invalid time report");
        sendAck();
        return;
    }

    mUsb.sendData(report);
}

void PanelClient::onSetDateTime(const PanelMessage &msg)
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

    sendAck();
}

SString<256> PanelClient::createDateTimeReport(const DateTime &dateTime) const
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

void PanelClient::onNetworkState()
{
    JsonObject json;
    json.add("id", PanelMessage::NetworkState);

    const bool connected = mWifi->isConnected();
    json.add("s", connected ? 1 : 0);
    if (connected) {
        json.add("ssid", mWifi->getSsid().c_str());
    }

    mUsb.sendData(json.dump());
}

void PanelClient::onNetworkConnect(const PanelMessage &msg)
{
    SendAckGuard g(mUsb);

    const SString<256> ssid = msg.getString("s");
    const SString<256> pass = msg.getString("p");

    if (ssid.empty() || pass.empty()) {
        mWifi->forgetNetwork();
        return;
    }

    if (!mWifi->connectNetwork(ssid.c_str(), pass.c_str())) {
        LOG_ERROR("Unable to connect to %s", ssid.c_str());
        return;
    }

    LOG("Successfully connected to %s", ssid.c_str());
}

void PanelClient::onNetworkLastConnect() 
{
    if (!mWifi->connecToLastNetwork()) {
        LOG_ERROR("Unable to connect to last network");
    } else {
        LOG("Successfully connected to %s", mWifi->getSsid().c_str());
    }

    sendAck();
}

void PanelClient::onNetworkDisconnect() 
{
    mWifi->disconnectNetwork();
    sendAck();
}

void PanelClient::onNtpState() 
{
    JsonObject json;
    json.add("id", PanelMessage::NtpState);
    json.add("url", mClock->getNtpServer().c_str());
    json.add("t", mClock->getTimeZone());

    mUsb.sendData(json.dump());
}

void PanelClient::onNtpSync()
{
    mClock->syncNtp();
    sendAck();
}

void PanelClient::onSetTimezone(const PanelMessage &msg) 
{
    SendAckGuard g(mUsb);

    int timezone = msg.getInt("t", 15);

    if (timezone > 12 || timezone < -12) {
        LOG_ERROR("Time zone value out of range: %i", timezone);
        return;
    }

    LOG("Setting time zone: %i", timezone);

    mClock->setTimeZone(int8_t(timezone));
}

void PanelClient::onSetServer(const PanelMessage &msg)
{
    SendAckGuard g(mUsb);
    
    SString<256> url = msg.getString("url");
    if (url.empty()) {
        LOG_ERROR("Empty ntp url");
        return;
    }

    LOG("Setting ntp url: %s", url.c_str());

    mClock->setNtpServer(SString<128>(url.data(), url.size()));
}

void PanelClient::onVersion()
{
    PanelMessage m(PanelMessage::VersionInfo);
    m.add("f", Version::getString());

    const ESP8266::Version espVersion = mWifi->getEspVersion();
    if (!espVersion.at.empty()) {
        m.add("at", espVersion.at.c_str());
    }

    if (!espVersion.sdk.empty()) {
        m.add("sdk", espVersion.sdk.c_str());
    }

    mUsb.sendData(m.toString());
}

void PanelClient::onDndState() 
{
    PanelMessage m(PanelMessage::DndState);
    m.add("s", Settings::isDndEnabled());

    const uint32_t start = Settings::getDndStart(25);
    const uint32_t end = Settings::getDndEnd(25);

    if (start < 24) {
        m.add("sh", start);
    }

    if (end < 24) {
        m.add("eh", end);
    }

    mUsb.sendData(m.toString());
}

void PanelClient::onSetDnd(const PanelMessage &msg)
{
    SendAckGuard g(mUsb);

    uint32_t start = msg.getInt("sh", 25);
    uint32_t end = msg.getInt("eh", 25);

    if (start > 23 || end > 23) {
        LOG_ERROR("Inavalid dnd hours");
        return;
    }

    LOG("Set DND from %i to %i", start, end);

    Settings::setDndStart(start);
    Settings::setDndStart(end);
}

void PanelClient::onEnableDnd(const PanelMessage &msg) 
{
    bool state = msg.getBool("s");
    LOG("%s DND", state ? "Enable" : "Disable");
    Settings::enableDnd(state);
    sendAck();
}
