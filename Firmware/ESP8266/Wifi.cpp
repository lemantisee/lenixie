#include "Wifi.h"

#include "Settings.h"

#include "Logger.h"

namespace {
// constexpr uint32_t checkConnectionPeriodMs = 30 * 60 * 1000; // 30 min
constexpr uint32_t checkConnectionPeriodMs = 1 * 60 * 1000; // 1 min
} // namespace

bool Wifi::init(USART_TypeDef *usart, uint32_t baudrate)
{
    if (!mEsp.init(usart, baudrate)) {
        return false;
    }

    connecToLastNetwork();

    return true;
}

void Wifi::process() 
{
    if (mEsp.getMode() != ESP8266::Station && mEsp.getMode() != ESP8266::StationAndSoftAP) {
        return;
    }

    checkConnection();
}

void Wifi::onConnect(std::function<void()> func) 
{
    mOnConnect = std::move(func);
}

bool Wifi::connectNetwork(const char *ssid, const char *password)
{
    if (!mEsp.connectToAp(ssid, password)) {
        return false;
    }

    mWasConnectedToNetwork = true;

    Settings::setWifiSSID(ssid);
    Settings::setWifiPassword(password);
    if (mOnConnect) {
        mOnConnect();
    }

    return true;
}

void Wifi::disconnectNetwork()
{
    mWasConnectedToNetwork = false;
    mEsp.disconnectFromAp();
}

void Wifi::forgetNetwork()
{
    Settings::setWifiSSID("");
    Settings::setWifiPassword("");
    disconnectNetwork();
}

bool Wifi::connectToServerUDP(const char *host, uint16_t port)
{
    if (!isConnected()) {
        return false;
    }

    return mEsp.connectToServerUDP(host, port);
}

bool Wifi::sendUDPpacket(const char *msg, uint16_t size)
{
    if (!isConnected()) {
        return false;
    }

    return mEsp.sendUDPpacket(msg, size);
}

bool Wifi::getData(uint8_t *buffer, uint8_t size) { return mEsp.getData(buffer, size); }

SString<128> Wifi::getSsid() const
{
    return Settings::getWifiSSID("");
}

bool Wifi::isConnected()
{
    switch (mEsp.getConnectionStatus()) {
    case ESP8266::GotIpStatus:
    case ESP8266::ConnectedStatus: return true;
    default: break;
    }

    return false;
}

ESP8266::Version Wifi::getEspVersion() { return mEsp.getVersion(); }

bool Wifi::connecToLastNetwork()
{
    const SString<128> ssid = Settings::getWifiSSID("");
    if (ssid.empty()) {
        LOG("No ssid");
        return false;
    }

    const SString<128> password = Settings::getWifiPassword("");
    if (password.empty()) {
        LOG("No wifi password");
        return false;
    }

    LOG("Connecting to wifi network %s", ssid.c_str());
    if (!connectNetwork(ssid.c_str(), password.c_str())) {
        LOG("Unable to connect to network");

        return false;
    }

    return true;
}

void Wifi::checkConnection() 
{
    if (!mWasConnectedToNetwork) {
        return;
    }

    if (mLastConnectionCheck + checkConnectionPeriodMs > HAL_GetTick()) {
        return;
    }

    mLastConnectionCheck = HAL_GetTick();

    if (isConnected()) {
        return;
    }

    LOG("Reconnecting to last wifi");

    connecToLastNetwork();
}
