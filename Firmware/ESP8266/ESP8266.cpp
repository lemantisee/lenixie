#include "ESP8266.h"

#include <cstring>

#include "SString.h"
#include "Logger.h"
#include "EspAtCommand.h"
#include "WifiCredentials.h"
#include "Uart.h"

namespace {
constexpr uint32_t checkConnectionPeriodMs = 30 * 60 * 1000; // 30 min
constexpr uint32_t obtainSSIDPeriodMs = 1000; // 30 min
} // namespace

bool ESP8266::init(Uart *uart)
{
    mUart = uart;
    mUart->onReceive(
        [this](const SString<64> &data) { mBuffer.append(data.c_str(), data.size()); });

    if (!enableEcho(false)) {
        LOG("Unable to echo");
        return false;
    }

    printVersion();

    if (!enableMultipleConnections(false)) {
        LOG("enableMultipleConnections fails");
        return false;
    }

    if (!setMode(Station)) {
        LOG("setMode fails");
        return false;
    }

    // HAL_Delay(2000);

    // obtainSSID();

    return true;
}

void ESP8266::process()
{
    if (mMode != Station && mMode != StationAndSoftAP) {
        return;
    }

    checkConnection();
    checkSSID();
}

void ESP8266::sendCommand(const char *cmd, bool sendEnd)
{
    mBuffer.clear();
    mUart->send(cmd, 100);
    if (sendEnd) {
        const char *cmdEnd = "\r\n";
        mUart->send(cmdEnd, 100);
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

    EspAtCommand cmd("AT+CWMODE_DEF=");
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

    LOG("Answer failed with buffer %s", mBuffer.c_str());
    mBuffer.clear();
    return false;
}

bool ESP8266::connectNetwork(const char *ssid, const char *password)
{
    if (!setMode(ESP8266::Station)) {
        return false;
    }

    EspAtCommand cmd("AT+CWJAP_DEF=");
    cmd.add(ssid).add(password);

    sendCommand(cmd);
    const bool ok = waitForAnswer("OK", 10000);
    if (ok) {
        mConnectedSSID = ssid;
        mConnectedPassword = password;
    }

    return ok;
}

bool ESP8266::test()
{
    sendCommand("AT", true);
    return waitForAnswer("OK", 1000);
}

bool ESP8266::connectToServerUDP(const char *host, uint16_t port)
{
    if (!isConnected()) {
        return false;
    }

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
    if (!isConnected()) {
        return false;
    }

    EspAtCommand cmd("AT+CIPSEND=");
    cmd.add(size);

    sendCommand(cmd);
    if (!waitForAnswer(">", 1000)) {
        return false;
    }

    mBuffer.clear();
    mUart->send((uint8_t *)msg, size, 100);

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

const SString<65> &ESP8266::getSsid() const
{
    return mConnectedSSID;
}

bool ESP8266::switchToAP()
{
    LOG("Starting AP");
    if (!setMode(ESP8266::SoftAP)) {
        return false;
    }

    if (!setAP(WifiCredentials::defaultApSsid(), WifiCredentials::defaultApPassword())) {
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

void ESP8266::obtainSSID()
{
    if (!isConnected()) {
        return;
    }

    uint8_t tries = 2;
    while (tries > 0) {
        --tries;

        sendCommand("AT+CWJAP_DEF?", true);
        if (!waitForAnswer("OK", 3000)) {
            continue;
        }
        //answer: +CWJAP:<ssid>, <bssid>, <channel>, <rssi>
        std::optional<uint32_t> posStartOpt = mBuffer.find(":");
        std::optional<uint32_t> posEndOpt = mBuffer.find(",");

        if (!posStartOpt || !posEndOpt) {
            LOG("Error: %s", mBuffer.c_str());
            break;
        }

        const size_t ssidSize = *posEndOpt - (*posStartOpt + 1);

        mConnectedSSID = SString<65>(mBuffer.c_str() + *posStartOpt + 1, ssidSize);
        mConnectedSSID.removeSymbol('\"');
        if (!mConnectedSSID.empty()) {
            break;
        }
    }
}

void ESP8266::printVersion() 
{
    EspAtCommand cmd("AT+GMR");
    sendCommand(cmd);
    waitForAnswer("OK", 1000);

    LOG("%s", mBuffer.c_str());
}

void ESP8266::checkConnection() 
{
    if (mLastConnectionCheck + checkConnectionPeriodMs > HAL_GetTick()) {
        return;
    }

    mLastConnectionCheck = HAL_GetTick();

    if (isConnected()) {
        return;
    }

    if (!connectNetwork(mConnectedSSID.c_str(), mConnectedPassword.c_str())) {
        LOG("Unable to connect to previuos wifi");
        return;
    }

    LOG("Reconnected to previuos wifi");
}

void ESP8266::checkSSID()
{
    if (!mConnectedSSID.empty()) {
        mLastSSIDCheck = 0;
        return;
    }

    if (mLastSSIDCheck + obtainSSIDPeriodMs > HAL_GetTick()) {
        return;
    }

    mLastSSIDCheck = HAL_GetTick();

    if (isConnected()) {
        obtainSSID();
    }
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

bool ESP8266::isConnected()
{
    if (mMode != Station && mMode != StationAndSoftAP) {
        return false;
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
        return false;
    }

    const uint32_t pos = *posOpt + std::strlen(statusStr);

    SString<1> statusStateStr(&mBuffer[pos], statusSymbolSize);
    ConnectionStatus status = ConnectionStatus(statusStateStr.toInt());

    switch (status)
    {
    case GotIpStatus:
    case ConnectedStatus:
        return true;
    default:
        break;
    }

    return false;
}
