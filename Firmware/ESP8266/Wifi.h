#pragma once

#include "ESP8266.h"

class Wifi
{
public:
    Wifi() = default;

    bool init(USART_TypeDef *usart, uint32_t baudrate);
    void process();

    void onConnect(std::function<void()> func);

    bool connectNetwork(const char *ssid, const char *password);
    bool connecToLastNetwork();
    void disconnectNetwork();
    void forgetNetwork();

    bool connectToServerUDP(const char *host, uint16_t port);
    bool sendUDPpacket(const char *msg, uint16_t size);
    bool getData(uint8_t *buffer, uint8_t size);

    SString<128> getSsid() const;
    bool isConnected();

private:
    void checkConnection();

    ESP8266 mEsp;
    uint32_t mLastConnectionCheck = 0;
    std::function<void()> mOnConnect;
    bool mWasConnectedToNetwork = false;
};
