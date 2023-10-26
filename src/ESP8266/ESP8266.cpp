#include "ESP8266.h"

#include <cstring>

#include "SString.h"
#include "Logger.h"
#include "EspAtCommand.h"
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

    Logger::log("Reseting wifi");
    reset();

    sendCommand("ATE0", true); //echo off
    waitForAnswer("OK", 1000);

    test();

    EspAtCommand cmd("AT+CIPMUX=");
    cmd.add(uint32_t(0));
    sendCommand(cmd);
    waitForAnswer("OK", 2000);

    setMode(Station);

    Logger::log("Wifi inited");
}

void ESP8266::sendCommand(const char *cmd, bool sendEnd) {
    mBuffer.clear();
    HAL_UART_Transmit(&mUart, (uint8_t *)cmd, std::strlen(cmd), 100);
    if (sendEnd) {
        const char *cmdEnd = "\r\n";
        HAL_UART_Transmit(&mUart, (uint8_t *)cmdEnd, std::strlen(cmdEnd), 100);
    }
}

void ESP8266::sendCommand(const EspAtCommand &cmd)
{
    sendCommand(cmd.string(), true);
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

bool ESP8266::setMode(Mode mode)
{
    if (mMode == mode) {
        return true;
    }

    mMode = mode;

    EspAtCommand cmd("AT+CWMODE=");
    cmd.add(mMode);

    switch (mMode)
    {
    case Station:
        Logger::log("Sending to ESP AT+CWMODE=1");
        break;
    case SoftAP:
        Logger::log("Sending to ESP AT+CWMODE=2");
        break;
    case StationAndSoftAP:
        Logger::log("Sending to ESP AT+CWMODE=3");
        break;
    default:
        return false;
    }
    
    sendCommand(cmd);
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
            return true;
        }

        asm("nop");
    }

    Logger::log("waitForAnswer failed with buffer");
    Logger::log(mBuffer.c_str());
    mBuffer.clear();
    return false;
}

bool ESP8266::connectNetwork(const char* ssid, const char* password) {
    if (!setMode(ESP8266::Station)) {
        return false;
    }

    Logger::log("Connecting to wifi network with command");

    EspAtCommand cmd("AT+CWJAP_CUR=");
    cmd.add(ssid).add(password);
    Logger::log(cmd.string());

    sendCommand(cmd);
    return waitForAnswer("OK", 20000);
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
    uint16_t localPort = 3210;
    uint16_t entity = 0; //destination peer entity of UDP will not change
    EspAtCommand cmd("AT+CIPSTART=");
    cmd.add("UDP").add(host).add(port).add(localPort).add(entity);

    sendCommand(cmd);
    return waitForAnswer("OK", 1000, "ALREADY");
}

bool ESP8266::sendUDPpacket(const char* msg, uint16_t size)
{
    EspAtCommand cmd("AT+CIPSEND=");
    cmd.add(size);

    sendCommand(cmd);
    if (!waitForAnswer(">", 1000)) {
        return false;
    }

    mBuffer.clear();
    HAL_UART_Transmit(&mUart, (uint8_t*)msg, size, 100);

    return waitForAnswer("OK", 2000);
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

bool ESP8266::getData(uint8_t *buffer, uint8_t size)
{
    const uint8_t espHeaderSize = 8;
    SString<12> answer;
    answer.append("+IPD,").appendNumber(size);

    if (!waitForAnswer(answer.c_str(), 2000)){
        return false;
    }

    HAL_Delay(1000);
    if (mBuffer.capacity() < 48 + espHeaderSize) {
        SString<20> str;
        str.append("buffer capacity ").appendNumber(mBuffer.capacity());
        Logger::log(str.c_str());
        return false;
    }
    char *ptr = strstr((char *)mBuffer.data(), answer.c_str());
    std::memcpy(buffer, ptr + espHeaderSize, size);
    return true;
}

bool ESP8266::switchToAP()
{
    Logger::log("Starting AP");
    if (!setMode(ESP8266::SoftAP)) {
        return false;
    }

	if (!setAP(WifiCredentials::defaultApSsid(), WifiCredentials::defaultApPassword())) {
        return false;
    }

    return connectToServerUDP("192.168.4.255", 1111);
}

bool ESP8266::setAP(const char *ssid, const char *pass) {
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
    EspAtCommand cmd("AT+CIPAP=");
    cmd.add(ip);

    sendCommand(cmd);
    return waitForAnswer("OK", 5000);
}

void ESP8266::reset() {
    sendCommand("AT+RST", true);
    HAL_Delay(10000);
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
