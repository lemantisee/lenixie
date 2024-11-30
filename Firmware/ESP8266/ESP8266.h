#pragma once

#include "Uart.h"

class EspAtCommand;

class ESP8266
{
public:
    enum Mode { Unknown = 0, Station = 1, SoftAP = 2, StationAndSoftAP = 3 };

    enum ConnectionStatus {
        GotIpStatus = 2,
        ConnectedStatus = 3,
        DisconnectedStatus = 4,
        WifiConnectionFail = 5
    };

    ESP8266() = default;

    bool init(USART_TypeDef *usart, uint32_t baudrate);

    bool connectToAp(const char *ssid, const char *password);
    bool disconnectFromAp();

    bool setAPip(const char *ip);

    bool switchToAP(const char *ssid, const char *password);

    bool connectToServerUDP(const char *host, uint16_t port);
    bool sendUDPpacket(const char *msg, uint16_t size);

    bool SendString(const char *str);
    bool SendString(const char *str, const char *ip, uint16_t port);
    bool hasIncomeData();
    bool getData(uint8_t *buffer, uint8_t size);
    Mode getMode() const;
    ConnectionStatus getConnectionStatus();

private:
    enum Encryption {
        Open = 0,
        WPA_PSK = 2,
        WPA2_PSK = 3,
        WPA_WPA2_PSK = 4,
    };

    bool setMode(Mode mode);
    bool setAP(const char *ssid, const char *pass);
    void sendCommand(const char *cmd, bool sendEnd = false);
    void sendCommand(const EspAtCommand &cmd);
    bool waitForAnswer(const char *answer1, uint32_t timeoutMs, const char *answer2 = nullptr);
    bool test();
    bool reset();
    void closeCurrentConnection();
    bool enableEcho(bool state);
    bool enableMultipleConnections(bool state);
    bool enableAutoconnection(bool state);
    void printVersion();

    Uart mUart;
    SString<256> mBuffer;
    Mode mMode = Unknown;
};
