#pragma once

#include "stm32f1xx.h"

#include "SString.h"

class EspAtCommand;
class Uart;

class ESP8266
{
public:
    ESP8266() = default;

    bool init(Uart *uart);
    void process();

    bool isConnected();
    bool connectNetwork(const char *ssid, const char *password);

    bool setAPip(const char *ip);

    bool switchToAP();

    bool connectToServerUDP(const char *host, uint16_t port);
    bool sendUDPpacket(const char *msg, uint16_t size);

    bool SendString(const char *str);
    bool SendString(const char *str, const char *ip, uint16_t port);
    bool hasIncomeData();
    uint8_t *getIncomeData();
    bool getData(uint8_t *buffer, uint8_t size);

private:
    enum Encryption {
        Open = 0,
        WPA_PSK = 2,
        WPA2_PSK = 3,
        WPA_WPA2_PSK = 4,
    };
    enum Mode { Unknown = 0, Station = 1, SoftAP = 2, StationAndSoftAP = 3 };

    bool setMode(Mode mode);
    bool setAP(const char *ssid, const char *pass);
    void sendCommand(const char *cmd, bool sendEnd = false);
    void sendCommand(const EspAtCommand &cmd);
    bool waitForAnswer(const char *answer1, uint16_t timeout, const char *answer2 = nullptr);
    bool test();
    bool reset();
    void closeCurrentConnection();
    bool enableEcho(bool state);
    bool enableMultipleConnections(bool state);

    Uart *mUart = nullptr;
    SString<256> mBuffer;
    Mode mMode = Unknown;
    uint32_t mLastConnectionCheck = 0;
    SString<65> mStationSSIDWasConnected;
    SString<65> mStationPswWasConnected;
};
