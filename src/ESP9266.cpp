#include "ESP9266.h"

#include <cstring>

#include "SString.h"
#include "Logger.h"

namespace
{
    ESP9266 *wifiInstance = nullptr;
} // namespace


void ESP9266::init(USART_TypeDef *usart, uint32_t baudrate)
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
    HAL_UART_RegisterRxEventCallback(&mUart, ESP9266::uartReceiveCallback);

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

void ESP9266::sendCommand(const char *cmd, bool sendEnd) {
    mBuffer.clear();
    HAL_UART_Transmit(&mUart, (uint8_t *)cmd, std::strlen(cmd), 100);
    if (sendEnd) {
        endCommand();
    }
}

void ESP9266::uartInterrupt()
{
    HAL_UART_IRQHandler(&mUart);
}

void ESP9266::startReadUart()
{
    mInputBuffer.clear();
    HAL_UARTEx_ReceiveToIdle_IT(&mUart, (uint8_t *)mInputBuffer.data(), mInputBuffer.size());
}

bool ESP9266::setMode(espMode_t mode)
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

bool ESP9266::waitForAnswer(const char *answer1, uint16_t timeout, const char *answer2)
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

bool ESP9266::connectNetwork(const char* ssid, const char* password) {
    setMode(ESP9266::STAMode);
    Logger::log("Connecting to wifi network");
    sendCommand("AT+CWJAP=");
    sendCommand("\"");
    sendCommand(ssid);
    sendCommand("\"");
    sendCommand(",");
    sendCommand("\"");
    sendCommand(password);
    sendCommand("\"", true);
    bool result = waitForAnswer("OK", 20000);

    Logger::log(result ? "Connected to wifi network" : "Not connected to wifi network");

    return result;

    /*
    sendCommand("AT+CIPMUX=0");
    endCommand();
    waitForAnswer("OK");
    sendCommand("AT+CIPMODE=0");
    endCommand();
    waitForAnswer("OK");
    */
}

void ESP9266::endCommand() {
    const char *str = "\r\n";
    HAL_UART_Transmit(&mUart, (uint8_t *)str, std::strlen(str), 100);
    mBuffer.clear();
}

bool ESP9266::test() {
    Logger::log("Send command AT");
    sendCommand("AT", true);
    return waitForAnswer("OK", 1000);
}

bool ESP9266::connectToServerUDP(const char* host, uint16_t port)
{
    //AT+CIPSTART="TCP","192.168.0.65",333
    //"UDP", "0", 0, 1025, 2
    std::array<char, 6> buff;
    snprintf(buff.data(), 6, "%d", port);
    sendCommand("AT+CIPSTART=\"UDP\",");
    sendCommand("\"");
    sendCommand(host);
    sendCommand("\"");
    sendCommand(",");
    sendCommand(buff.data());
    sendCommand(",");
    sendCommand("1112");
    sendCommand(",0", true);
    return waitForAnswer("OK", 1000, "ALREADY");
}

bool ESP9266::sendUDPpacket(const char* msg, uint16_t size)
{
    mBuffer.clear();
    sendCommand("AT+CIPSEND=48", true);
    waitForAnswer(">", 1000);
    mBuffer.clear();
    sendData((uint8_t*)msg, size);
    return waitForAnswer("OK", 2000);
}

bool ESP9266::sendUDPpacket(const char* msg, const char* size)
{
    sendCommand("AT+CIPSEND=");
    sendCommand(size, true);
    waitForAnswer(">", 1000);
    sendCommand(msg, true);
    return true;
}

void ESP9266::sendData(uint8_t *data, uint16_t size) {
    HAL_UART_Transmit(&mUart, data, size, 100);
}

bool ESP9266::getIP() {
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

uint8_t * ESP9266::getData(uint8_t size) {
    SString<12> answer;
    answer.append("+IPD,").appendNumber(size);

    if (!waitForAnswer(answer.c_str(), 2000)){
        return nullptr;
    }

    HAL_Delay(1000);
    char *ptr = strstr((char *)mBuffer.data(), answer.c_str());
    return ((uint8_t*)ptr + 8);
}

void ESP9266::switchToAP()
{
    Logger::log("Starting AP");
	mBuffer.clear();
    setMode(ESP9266::SoftAP);
	mBuffer.clear();
	setAP("NxC2", "NxCPass12", "192.168.4.1");
	mBuffer.clear();
    connectToServerUDP("192.168.4.255", 1111);
	mBuffer.clear();
}

bool ESP9266::setAP(const char *ssid, const char *pass, const char *) {
    //AT + CWSAP = "Test2", "habrahabr", 10, 4
    //AT + CWSAP ?
    //sendCommand("AT+CWSAP?");
    //AT + CIPAP_CUR = "192.168.5.1"

    //waitForAnswer("OK", 5000);
    sendCommand("AT+CWSAP=");
    sendCommand("\"");
    sendCommand(ssid);
    sendCommand("\"");
    sendCommand(",");
    sendCommand("\"");
    sendCommand(pass);
    sendCommand("\"");
    sendCommand(",");
    sendCommand("10,3", true);
    return waitForAnswer("OK", 5000);
}

void ESP9266::SendString(const char *str) {
    mBuffer.clear();
    HAL_Delay(1000);
    uint8_t strSize = strlen(str);
    char buffSize[4];
    memset(buffSize, 0, 4);
    snprintf(buffSize, 4, "%d", strSize);

    sendCommand("AT+CIPSEND=");
    sendCommand(buffSize, true);
    waitForAnswer(">", 1000);
    Logger::log("Sending data:");
    Logger::log(str);
    sendCommand(str);
    waitForAnswer("SEND OK", 2000);
}

void ESP9266::SendString(const char *str, const char *ip, uint16_t port) {
    mBuffer.clear();
    uint8_t strSize = strlen(str);
    char buffSize[4];
    memset(buffSize, 0, 4);
    snprintf(buffSize, 4, "%d", strSize);

    char portStr[6];
    memset(portStr, 0, 6);
    snprintf(portStr, 6, "%d", port);

    sendCommand("AT+CIPSEND=");
    sendCommand(buffSize);
    sendCommand(",");
    sendCommand("\"");
    sendCommand(ip);
    sendCommand("\"");
    sendCommand(",");
    sendCommand(portStr, true);
    waitForAnswer(">", 1000);
    Logger::log("Sending data:");
    Logger::log(str);
    sendCommand(str);
    waitForAnswer("SEND OK", 2000);
}

uint8_t * ESP9266::getIncomeData() {
    char *ptr = strchr((char *)mBuffer.data(), ':');
    return ((uint8_t*)ptr + 1);
}

bool ESP9266::hasIncomeData()
{
    return mBuffer.contains("+IPD");
}

void ESP9266::clearBuffer()
{
    mBuffer.clear();
}

void ESP9266::closeCurrentConnection() {
    Logger::log("Closing current connection");
    sendCommand("AT+CIPCLOSE", true);
    waitForAnswer("OK", 1000);
}

bool ESP9266::setAPip(const char *ip) {
    sendCommand("AT+CIPAP=");
    sendCommand("\"");
    sendCommand(ip);
    sendCommand("\"", true);
    return waitForAnswer("OK", 5000);
}

void ESP9266::reset() {
    sendCommand("AT+RST", true);
    //endCommand();
    HAL_Delay(10000);
    //waitForAnswer("OK", 10000);
    Logger::log("Wifi reset complete");
}

void ESP9266::uartReceiveCallback(UART_HandleTypeDef *uart, uint16_t size)
{
    if(uart != &wifiInstance->mUart) {
        return;
    }

    wifiInstance->mBuffer.append(wifiInstance->mInputBuffer.data(), size);
    wifiInstance->startReadUart();
}

bool ESP9266::isConnected() {
    sendCommand("AT+CIPSTATUS", true);
    return waitForAnswer("STATUS:2", 3000, "STATUS:3");
}
