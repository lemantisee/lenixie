#include "NetworkSession.h"

#include "ESP8266.h"
#include "Logger.h"

void NetworkSession::handle(const PanelMessage &msg)
{
    switch (msg.getCmd()) {
    case PanelMessage::GetNetworkState: onState(); break;
    case PanelMessage::ConnectToWifi: onConnect(msg); break;
    default: toNext(msg); break;
    }
}

void NetworkSession::setWifi(ESP8266 *wifi) { mWifi = wifi; }

void NetworkSession::onState()
{
    JsonObject json;
    json.add("id", PanelMessage::NetworkState);

    const bool connected = mWifi->isConnected();
    json.add("s", connected ? 1 : 0);
    if (connected) {
        json.add("ssid", mWifi->getSsid().c_str());
    }

    send(json.dump());
}

void NetworkSession::onConnect(const PanelMessage &msg)
{
    SString<256> ssid = msg.getString("s");
    if (ssid.empty()) {
        LOG_ERROR("Empty ssid");
        return;
    }

    SString<256> pass = msg.getString("p");
    if (pass.empty()) {
        LOG_ERROR("Empty password");
        return;
    }

    if (!mWifi->connectNetwork(ssid.c_str(), pass.c_str())) {
        LOG_ERROR("Unable to connect to %s", ssid.c_str());
        return;
    }

    LOG("Successfully connected to %s", ssid.c_str());
}
