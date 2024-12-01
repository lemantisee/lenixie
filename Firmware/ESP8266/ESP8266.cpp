#include "ESP8266.h"

#include <cstring>

#include "SString.h"
#include "Logger.h"
#include "EspAtCommand.h"

bool ESP8266::init(USART_TypeDef *usart, uint32_t baudrate)
{
    mUart.onReceive(
        [this](const SString<64> &data) { mBuffer.append(data.c_str(), data.size()); });

    if (!mUart.init(usart, baudrate)) {
        LOG_ERROR("Unable to init uart");
        return false;
    }

    if (!enableEcho(false)) {
        LOG_ERROR("Unable to disable echo");
        return false;
    }

    if (!enableAutoconnection(false)) {
        LOG_ERROR("Unable to disable autoconnection");
        return false;
    }

    if (!enableMultipleConnections(false)) {
        LOG_ERROR("Unable to disable multiple connections");
        return false;
    }

    if (!setMode(Station)) {
        LOG_ERROR("Unable to station mode");
        return false;
    }

    return true;
}

void ESP8266::sendCommand(const char *cmd, bool sendEnd)
{
    mBuffer.clear();
    mUart.send(cmd, 100);
    if (sendEnd) {
        const char *cmdEnd = "\r\n";
        mUart.send(cmdEnd, 100);
    }
}

void ESP8266::sendCommand(const EspAtCommand &cmd) { sendCommand(cmd.string(), true); }

bool ESP8266::setMode(Mode mode)
{
    if (mMode == mode) {
        return true;
    }

    if (mode == Unknown) {
        return false;
    }

    EspAtCommand cmd("AT+CWMODE_CUR=");
    cmd.add(mode);

    sendCommand(cmd);
    if (!waitForAnswer("OK", 5000)) {
        return false;
    }

    mMode = mode;
    return true;
}

bool ESP8266::waitForAnswer(const char *answer1, uint32_t timeoutMs, const char *answer2)
{
    timeoutMs += HAL_GetTick();
    while ((HAL_GetTick() < timeoutMs)) {
        bool checkAnswer = mBuffer.contains(answer1);
        if (answer2) {
            checkAnswer = checkAnswer || mBuffer.contains(answer2);
        }

        if (checkAnswer) {
            return true;
        }
    }

    LOG("Answer failed: %s", mBuffer.c_str());
    mBuffer.clear();
    return false;
}

bool ESP8266::test()
{
    sendCommand("AT", true);
    return waitForAnswer("OK", 1000);
}

bool ESP8266::connectToServerUDP(const char *host, uint16_t port)
{
    //AT+CIPSTART="TCP","192.168.0.65",333
    //"UDP", "0", 0, 1025, 2
    uint16_t localPort = 3210;
    uint16_t entity = 0; //destination peer entity of UDP will not change
    EspAtCommand cmd("AT+CIPSTART=");
    cmd.add("UDP").add(host).add(port).add(localPort).add(entity);

    sendCommand(cmd);
    return waitForAnswer("OK", 1000, "ALREADY");
}

bool ESP8266::sendUDPpacket(const char *msg, uint16_t size)
{
    EspAtCommand cmd("AT+CIPSEND=");
    cmd.add(size);

    sendCommand(cmd);
    if (!waitForAnswer(">", 1000)) {
        return false;
    }

    mBuffer.clear();
    mUart.send((uint8_t *)msg, size, 100);

    return waitForAnswer("OK", 2000);
}

bool ESP8266::getData(uint8_t *buffer, uint8_t size)
{
    const uint8_t espHeaderSize = 8;
    SString<12> answer;
    answer.append("+IPD,").appendNumber(size);

    if (!waitForAnswer(answer.c_str(), 2000)) {
        return false;
    }

    HAL_Delay(1000);
    
    char *ptr = strstr((char *)mBuffer.data(), answer.c_str());
    std::memcpy(buffer, ptr + espHeaderSize, size);
    return true;
}

ESP8266::Mode ESP8266::getMode() const { return mMode; }

ESP8266::ConnectionStatus ESP8266::getConnectionStatus()
{
    if (mMode != Station && mMode != StationAndSoftAP) {
        return DisconnectedStatus;
    }

    sendCommand("AT+CIPSTATUS", true);

    const uint32_t timeout = 3000 + HAL_GetTick();
    const char *statusStr = "STATUS:";
    const uint32_t statusStrSize = std::strlen(statusStr);
    const uint8_t statusSymbolSize = 1;

    std::optional<uint32_t> posOpt = 0;

    while ((HAL_GetTick() < timeout)) {
        posOpt = mBuffer.find(statusStr);

        if (posOpt && mBuffer.size() >= statusStrSize + statusSymbolSize) {
            break;
        }
    }

    if (!posOpt) {
        return DisconnectedStatus;
    }

    const uint32_t pos = *posOpt + std::strlen(statusStr);

    SString<1> statusStateStr(&mBuffer[pos], statusSymbolSize);
    return ConnectionStatus(statusStateStr.toInt());
}

ESP8266::Version ESP8266::getVersion()
{
    EspAtCommand cmd("AT+GMR");
    sendCommand(cmd);
    waitForAnswer("OK", 1000);

    Version ver;
    ver.at = getTokenVersion(mBuffer, "AT version:");
    ver.sdk = getTokenVersion(mBuffer, "SDK version:");

    return ver;
}

bool ESP8266::switchToAP(const char *ssid, const char *password)
{
    LOG("Starting AP");
    if (!setMode(ESP8266::SoftAP)) {
        return false;
    }

    if (!setAP(ssid, password)) {
        return false;
    }

    return connectToServerUDP("192.168.4.255", 1111);
}

bool ESP8266::setAP(const char *ssid, const char *pass)
{
    //AT+CWSAP="ssid","password",10,4

    const uint8_t channel = 10;

    EspAtCommand cmd("AT+CWSAP=");
    cmd.add(ssid).add(pass).add(channel).add(WPA2_PSK);

    sendCommand(cmd);
    return waitForAnswer("OK", 5000);
}

bool ESP8266::SendString(const char *str)
{
    EspAtCommand cmd("AT+CIPSEND=");
    cmd.add(strlen(str));
    sendCommand(cmd);
    if (!waitForAnswer(">", 1000)) {
        return false;
    }

    sendCommand(str);
    return waitForAnswer("SEND OK", 2000);
}

bool ESP8266::SendString(const char *str, const char *ip, uint16_t port)
{
    EspAtCommand cmd("AT+CIPSEND=");
    cmd.add(strlen(str)).add(ip).add(port);

    sendCommand(cmd);
    if (!waitForAnswer(">", 1000)) {
        return false;
    }

    sendCommand(str);
    return waitForAnswer("SEND OK", 2000);
}

bool ESP8266::hasIncomeData() { return mBuffer.contains("+IPD"); }

void ESP8266::closeCurrentConnection()
{
    sendCommand("AT+CIPCLOSE", true);
    waitForAnswer("OK", 1000);
}

bool ESP8266::enableEcho(bool state)
{
    EspAtCommand cmd("ATE");
    cmd.add(state ? uint32_t(1) : uint32_t(0));
    sendCommand(cmd);

    return waitForAnswer("OK", 1000);
}

bool ESP8266::enableMultipleConnections(bool state)
{
    EspAtCommand cmd("AT+CIPMUX=");
    cmd.add(state ? uint32_t(1) : uint32_t(0));
    sendCommand(cmd);

    return waitForAnswer("OK", 2000);
}

bool ESP8266::enableAutoconnection(bool state)
{
    EspAtCommand cmd("AT+CWAUTOCONN=");
    cmd.add(state ? uint32_t(1) : uint32_t(0));
    sendCommand(cmd);

    return waitForAnswer("OK", 2000);
}

SString<64> ESP8266::getTokenVersion(const SString<256> &str, const char *token) const
{
    auto posStartOpt = str.find(token);
    if (!posStartOpt) {
        return {};
    }

    const uint32_t posStart = *posStartOpt + std::strlen(token);

    auto posEndOpt = str.find("(", posStart);
    if (!posEndOpt) {
        return {};
    }

    const uint32_t len = *posEndOpt - posStart;
    return SString<64>(str.c_str() + posStart, len);
}

bool ESP8266::connectToAp(const char *ssid, const char *password)
{
    if (!setMode(Station)) {
        return false;
    }

    EspAtCommand cmd("AT+CWJAP_CUR=");
    cmd.add(ssid).add(password);

    sendCommand(cmd);
    return waitForAnswer("OK", 10000);
}

bool ESP8266::disconnectFromAp()
{
    if (!setMode(Station)) {
        return false;
    }

    EspAtCommand cmd("AT+CWQAP");
    sendCommand(cmd);
    return waitForAnswer("OK", 3000);
}

bool ESP8266::setAPip(const char *ip)
{
    EspAtCommand cmd("AT+CIPAP=");
    cmd.add(ip);

    sendCommand(cmd);
    return waitForAnswer("OK", 5000);
}

bool ESP8266::reset()
{
    sendCommand("AT+RST", true);
    HAL_Delay(10000);
    return true;
}
