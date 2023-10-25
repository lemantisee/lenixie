#include "ESP8266.h"

#include <cstring>

#include "SString.h"
#include "Logger.h"
#include "WifiCredentials.h"

namespace
{
    ESP8266 *wifiInstance = nullptr;
} // namespace


void ESP8266::init(USART_TypeDef *usart, uint32_t baudrate)
{
#if USE_HAL_UART_REGISTER_CALLBACKS != 1
#error "Error. USART callback not enabled"
#endif
    wifiInstance = this;
    answerReady = false;
    mTimeout = false;
    mMode = STAMode;

    mUart.Instance = usart;
    mUart.Init.BaudRate = baudrate;
    mUart.Init.WordLength = UART_WORDLENGTH_8B;
    mUart.Init.StopBits = UART_STOPBITS_1;
    mUart.Init.Parity = UART_PARITY_NONE;
    mUart.Init.Mode = UART_MODE_TX_RX;
    mUart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    mUart.Init.OverSampling = UART_OVERSAMPLING_16;
    HAL_UART_Init(&mUart);
    HAL_UART_RegisterRxEventCallback(&mUart, ESP8266::uartReceiveCallback);

    startReadUart();

    HAL_Delay(500);

    sendCommand("ATE0", true); //echo off
    waitForAnswer("OK", 1000);

    reset();
    test();

    sendCommand("AT+CIPMUX=0", true);
    waitForAnswer("OK", 2000);

    Logger::log("Wifi inited");
}

void ESP8266::sendCommand(const char *cmd, bool sendEnd) {
    mBuffer.clear();
    HAL_UART_Transmit(&mUart, (uint8_t *)cmd, std::strlen(cmd), 100);
    if (sendEnd) {
        endCommand();
    }
}

void ESP8266::uartInterrupt()
{
    HAL_UART_IRQHandler(&mUart);
}

void ESP8266::startReadUart()
{
    mInputBuffer.clear();
    HAL_UARTEx_ReceiveToIdle_IT(&mUart, (uint8_t *)mInputBuffer.data(), mInputBuffer.size());
}

bool ESP8266::setMode(espMode_t mode)
{
    if (mMode == mode) {
        return true;
    }

    mMode = mode;
    switch (mMode)
    {
    case STAMode:
        Logger::log("Sending to ESP AT+CWMODE=1\n");
        sendCommand("AT+CWMODE=1", true);
        break;
    case SoftAP:
        Logger::log("Sending to ESP AT+CWMODE=2\n");
        sendCommand("AT+CWMODE=2", true);
        break;
    case STA_AP:
        Logger::log("Sending to ESP AT+CWMODE=3\n");
        sendCommand("AT+CWMODE=3", true);
        break;
    default:
        break;
    }
    
    return waitForAnswer("OK", 5000);
}

bool ESP8266::waitForAnswer(const char *answer1, uint16_t timeout, const char *answer2)
{
    timeout += HAL_GetTick();
    while ((HAL_GetTick() < timeout)) {
        bool checkAnswer = strstr(mBuffer.c_str(), answer1) != nullptr;
        if (answer2) {
            checkAnswer = checkAnswer || strstr(mBuffer.c_str(), answer2) != nullptr;
        }

        if (checkAnswer) {
            Logger::log(mBuffer.c_str());
            mBuffer.clear();
            return true;
        }

        asm("nop");
        //for (int i = 0; i < 20000; i++) asm("nop");

    }

    Logger::log("waitForAnswer failed with buffer");
    Logger::log(mBuffer.c_str());
    mBuffer.clear();
    return false;
}

bool ESP8266::connectNetwork(const char* ssid, const char* password) {
    if (!setMode(ESP8266::STAMode)) {
        return false;
    }

    Logger::log("Connecting to wifi network");

    SString<255> cmd;
    cmd.append("AT+CWJAP=");
    cmd.append("\"").append(ssid).append("\",");
    cmd.append("\"").append(password).append("\"");
    sendCommand(cmd.c_str(), true);

    return waitForAnswer("OK", 20000);

    // sendCommand("AT+CWJAP=");
    // sendCommand("\"");
    // sendCommand(ssid);
    // sendCommand("\"");
    // sendCommand(",");
    // sendCommand("\"");
    // sendCommand(password);
    // sendCommand("\"", true);
    // bool result = waitForAnswer("OK", 20000);

    // Logger::log(result ? "Connected to wifi network" : "Not connected to wifi network");

    // return result;

    /*
    sendCommand("AT+CIPMUX=0");
    endCommand();
    waitForAnswer("OK");
    sendCommand("AT+CIPMODE=0");
    endCommand();
    waitForAnswer("OK");
    */
}

void ESP8266::endCommand() {
    const char *str = "\r\n";
    HAL_UART_Transmit(&mUart, (uint8_t *)str, std::strlen(str), 100);
    mBuffer.clear();
}

bool ESP8266::test() {
    Logger::log("Send command AT");
    sendCommand("AT", true);
    return waitForAnswer("OK", 1000);
}

bool ESP8266::connectToServerUDP(const char* host, uint16_t port)
{
    //AT+CIPSTART="TCP","192.168.0.65",333
    //"UDP", "0", 0, 1025, 2
    const char *localPort = "3210";
    SString<255> cmd;
    cmd.append("AT+CIPSTART=\"UDP\",");
    cmd.append("\"").append(host).append("\",").appendNumber(port).append(",");
    cmd.appendNumber(port).append(",");
    cmd.append(localPort).append(",");
    cmd.appendNumber(0); //destination peer entity of UDP will not change

    sendCommand(cmd.c_str(), true);
    return waitForAnswer("OK", 1000, "ALREADY");

    // std::array<char, 6> buff;
    // snprintf(buff.data(), 6, "%d", port);
    // sendCommand("AT+CIPSTART=\"UDP\",");
    // sendCommand("\"");
    // sendCommand(host);
    // sendCommand("\"");
    // sendCommand(",");
    // sendCommand(buff.data());
    // sendCommand(",");
    // sendCommand("1112");
    // sendCommand(",0", true);
    // return waitForAnswer("OK", 1000, "ALREADY");
}

bool ESP8266::sendUDPpacket(const char* msg, uint16_t size)
{
    SString<255> cmd;
    cmd.append("AT+CIPSEND=").appendNumber(size);

    sendCommand(cmd.c_str(), true);
    if (!waitForAnswer(">", 1000)) {
        return false;
    }

    sendData((uint8_t*)msg, size);
    return waitForAnswer("OK", 2000);

    // sendCommand("AT+CIPSEND=48", true);
    // waitForAnswer(">", 1000);
    // mBuffer.clear();
    // sendData((uint8_t*)msg, size);
    // return waitForAnswer("OK", 2000);
}

bool ESP8266::sendUDPpacket(const char* msg, const char* size)
{
    SString<255> cmd;
    cmd.append("AT+CIPSEND=").append(size);

    sendCommand(cmd.c_str(), true);
    if (!waitForAnswer(">", 1000)) {
        return false;
    }

    sendCommand(msg, true);
    return true;

    // sendCommand("AT+CIPSEND=");
    // sendCommand(size, true);
    // waitForAnswer(">", 1000);
    // sendCommand(msg, true);
    // return true;
}

void ESP8266::sendData(uint8_t *data, uint16_t size) {
    mBuffer.clear();
    HAL_UART_Transmit(&mUart, data, size, 100);
}

bool ESP8266::getIP() {
    //AT + CIFSR
    memset(broadcastIP, 0, 17);
    memset(ipoctet1, 0, 4);
    memset(ipoctet2, 0, 4);
    memset(ipoctet3, 0, 4);
    memset(ipoctet4, 0, 4);
    sendCommand("AT+CIFSR", true);
    waitForAnswer("OK", 1000);
    char *str1 = strchr((char *)mBuffer.data(), '"');
    char *str2 = strchr(str1 + 1, '"');
    char iptmp[17];
    strncpy(iptmp, str1 + 1, str2 - str1);
    char *tmp = iptmp;
    strcpy(ipoctet1, strsep(&tmp, "."));
    strcpy(ipoctet2, strsep(&tmp, "."));
    strcpy(ipoctet3, strsep(&tmp, "."));
    strcpy(ipoctet4, strsep(&tmp, "."));
    strcat(broadcastIP, ipoctet1);
    strcat(broadcastIP, ".");
    strcat(broadcastIP, ipoctet2);
    strcat(broadcastIP, ".");
    strcat(broadcastIP, ipoctet3);
    strcat(broadcastIP, ".");
    strcat(broadcastIP, "255");
    return true;
}

uint8_t * ESP8266::getData(uint8_t size) {
    SString<12> answer;
    answer.append("+IPD,").appendNumber(size);

    if (!waitForAnswer(answer.c_str(), 2000)){
        return nullptr;
    }

    HAL_Delay(1000);
    char *ptr = strstr((char *)mBuffer.data(), answer.c_str());
    return ((uint8_t*)ptr + 8);
}

bool ESP8266::switchToAP()
{
    Logger::log("Starting AP");
    if (!setMode(ESP8266::SoftAP)) {
        return false;
    }

	if (!setAP(WifiCredentials::defaultApSsid, WifiCredentials::defaultApPassword)) {
        return false;
    }

    return connectToServerUDP("192.168.4.255", 1111);
}

bool ESP8266::setAP(const char *ssid, const char *pass) {
    //AT+CWSAP="ssid","password",10,4

    const uint8_t channel = 10;
    SString<255> cmd;
    cmd.append("AT+CWSAP=");
    cmd.append("\"").append(ssid).append("\",");
    cmd.append("\"").append(pass).append("\",");
    cmd.appendNumber(channel).append(",").appendNumber(WPA2_PSK);

    sendCommand(cmd.c_str(), true);
    return waitForAnswer("OK", 5000);
}

bool ESP8266::SendString(const char *str)
{
    SString<255> cmd;
    cmd.append("AT+CIPSEND=").appendNumber(strlen(str));
    sendCommand(cmd.c_str(), true);
    if (!waitForAnswer(">", 1000)) {
        return false;
    }

    sendCommand(str);
    return waitForAnswer("SEND OK", 2000);

    // HAL_Delay(1000);
    // uint8_t strSize = strlen(str);
    // char buffSize[4];
    // memset(buffSize, 0, 4);
    // snprintf(buffSize, 4, "%d", strSize);

    // sendCommand("AT+CIPSEND=");
    // sendCommand(buffSize, true);
    // waitForAnswer(">", 1000);
    // Logger::log("Sending data:");
    // Logger::log(str);
    // sendCommand(str);
    // waitForAnswer("SEND OK", 2000);
}

bool ESP8266::SendString(const char *str, const char *ip, uint16_t port)
{
    SString<255> cmd;
    cmd.append("AT+CIPSEND=").appendNumber(strlen(str)).append(",");
    cmd.append("\"").append(ip).append("\",").appendNumber(port);

    sendCommand(cmd.c_str(), true);
    if (!waitForAnswer(">", 1000)) {
        return false;
    }

    sendCommand(str);
    return waitForAnswer("SEND OK", 2000);

    // uint8_t strSize = strlen(str);
    // char buffSize[4];
    // memset(buffSize, 0, 4);
    // snprintf(buffSize, 4, "%d", strSize);

    // char portStr[6];
    // memset(portStr, 0, 6);
    // snprintf(portStr, 6, "%d", port);

    // sendCommand("AT+CIPSEND=");
    // sendCommand(buffSize);
    // sendCommand(",");
    // sendCommand("\"");
    // sendCommand(ip);
    // sendCommand("\"");
    // sendCommand(",");
    // sendCommand(portStr, true);
    // waitForAnswer(">", 1000);
    // Logger::log("Sending data:");
    // Logger::log(str);
    // sendCommand(str);
    // waitForAnswer("SEND OK", 2000);
}

uint8_t * ESP8266::getIncomeData() {
    char *ptr = strchr((char *)mBuffer.data(), ':');
    return ((uint8_t*)ptr + 1);
}

bool ESP8266::hasIncomeData()
{
    return mBuffer.contains("+IPD");
}

void ESP8266::clearBuffer()
{
    mBuffer.clear();
}

void ESP8266::closeCurrentConnection() {
    Logger::log("Closing current connection");
    sendCommand("AT+CIPCLOSE", true);
    waitForAnswer("OK", 1000);
}

bool ESP8266::setAPip(const char *ip) {
    SString<255> cmd;
    cmd.append("AT+CIPAP=").append("\"").append(ip).append("\"");

    sendCommand(cmd.c_str(), true);
    return waitForAnswer("OK", 5000);

    // sendCommand("AT+CIPAP=");
    // sendCommand("\"");
    // sendCommand(ip);
    // sendCommand("\"", true);
    // return waitForAnswer("OK", 5000);
}

void ESP8266::reset() {
    sendCommand("AT+RST", true);
    HAL_Delay(10000);
    //waitForAnswer("OK", 10000);
    Logger::log("Wifi reset complete");
}

void ESP8266::uartReceiveCallback(UART_HandleTypeDef *uart, uint16_t size)
{
    if(uart != &wifiInstance->mUart) {
        return;
    }

    wifiInstance->mBuffer.append(wifiInstance->mInputBuffer.data(), size);
    wifiInstance->startReadUart();
}

bool ESP8266::isConnected() {
    sendCommand("AT+CIPSTATUS", true);
    return waitForAnswer("STATUS:2", 3000, "STATUS:3");
}
