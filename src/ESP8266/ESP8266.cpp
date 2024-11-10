#include "ESP8266.h"

#include <cstring>

#include "SString.h"
#include "Logger.h"
#include "EspAtCommand.h"
#include "WifiCredentials.h"

namespace
{
    constexpr uint32_t checkConnectionPeriodMs = 30 * 60 * 1000; // 30 min
    ESP8266 *wifiInstance = nullptr;
} // namespace


bool ESP8266::init(USART_TypeDef *usart, uint32_t baudrate)
{
#if USE_HAL_UART_REGISTER_CALLBACKS != 1
#error "Error. USART callback not enabled"
#endif
    wifiInstance = this;

    if (!setupUart(usart, baudrate)) {
        LOG("Unable to setup uart");
        return false;
    }

    if (!startReadUart()) {
        LOG("Unable to start uart");
        return false;
    }

    HAL_Delay(500);

    // reset();

    if (!enableEcho(false)) {
        LOG("Unable to echo");
        return false;
    }

    if (!enableMultipleConnections(false)) {
        LOG("enableMultipleConnections fails");
        return false;
    }

    if (!setMode(Station)) {
        LOG("setMode fails");
        return false;
    }

    return true;
}

void ESP8266::process()
{
    if (mMode != Station && mMode != StationAndSoftAP) {
        return;
    }

    if (mLastConnectionCheck + checkConnectionPeriodMs > HAL_GetTick()) {
        return;
    }

    mLastConnectionCheck = HAL_GetTick();

    if (isConnected()) {
        return;
    }

    if (!connectNetwork(mStationSSIDWasConnected.c_str(), mStationPswWasConnected.c_str())) {
        LOG("Unable to connect to previuos wifi");
        return;
    }

    LOG("Reconnected to previuos wifi");
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

bool ESP8266::setupUart(USART_TypeDef *usart, uint32_t baudrate)
{
    mUart.Instance = usart;
    mUart.Init.BaudRate = baudrate;
    mUart.Init.WordLength = UART_WORDLENGTH_8B;
    mUart.Init.StopBits = UART_STOPBITS_1;
    mUart.Init.Parity = UART_PARITY_NONE;
    mUart.Init.Mode = UART_MODE_TX_RX;
    mUart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    mUart.Init.OverSampling = UART_OVERSAMPLING_16;

    if (HAL_UART_RegisterCallback(&mUart, HAL_UART_MSPINIT_CB_ID, ESP8266::uartInitCallback) != HAL_OK ) {
        return false;
    }

    if (HAL_UART_RegisterCallback(&mUart, HAL_UART_MSPDEINIT_CB_ID, ESP8266::uartDeinitCallback) != HAL_OK ) {
        return false;
    }

    if (HAL_UART_Init(&mUart) != HAL_OK ) {
        return false;
    }

    if (HAL_UART_RegisterRxEventCallback(&mUart, ESP8266::uartReceiveCallback) != HAL_OK ) {
        return false;
    }

    return true;
}

bool ESP8266::startReadUart()
{
    mInputBuffer.clear();
    return HAL_UARTEx_ReceiveToIdle_IT(&mUart, (uint8_t *)mInputBuffer.data(), mInputBuffer.capacity()) == HAL_OK;
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
        break;
    case SoftAP:
        break;
    case StationAndSoftAP:
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
        bool checkAnswer = mBuffer.contains(answer1);
        if (answer2) {
            checkAnswer = checkAnswer || mBuffer.contains(answer2);
        }

        if (checkAnswer) {
            return true;
        }
    }

    LOG("waitForAnswer failed with buffer %s", mBuffer.c_str());
    mBuffer.clear();
    return false;
}

bool ESP8266::connectNetwork(const char *ssid, const char *password)
{
    if (!setMode(ESP8266::Station)) {
        return false;
    }

    EspAtCommand cmd("AT+CWJAP_CUR=");
    cmd.add(ssid).add(password);
    LOG(cmd.string());

    sendCommand(cmd);
    const bool ok = waitForAnswer("OK", 20000);
    if (ok) {
        mStationSSIDWasConnected = ssid;
        mStationPswWasConnected = password;
    }

    return ok;
}

bool ESP8266::test() {
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

    char ipoctet1[4];
	char ipoctet2[4];
	char ipoctet3[4];
	char ipoctet4[4];

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
        LOG(str.c_str());
        return false;
    }
    char *ptr = strstr((char *)mBuffer.data(), answer.c_str());
    std::memcpy(buffer, ptr + espHeaderSize, size);
    return true;
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

bool ESP8266::setAPip(const char *ip) {
    EspAtCommand cmd("AT+CIPAP=");
    cmd.add(ip);

    sendCommand(cmd);
    return waitForAnswer("OK", 5000);
}

bool ESP8266::reset() {
    sendCommand("AT+RST", true);
    HAL_Delay(10000);
    return true;
}

void ESP8266::uartReceiveCallback(UART_HandleTypeDef *uart, uint16_t size)
{
    if(uart != &wifiInstance->mUart) {
        return;
    }

    wifiInstance->mBuffer.append(wifiInstance->mInputBuffer.data(), size);
    wifiInstance->startReadUart();
}

void ESP8266::uartInitCallback(UART_HandleTypeDef *huart)
{
    if (!wifiInstance) {
        return;
    }

    if (huart->Instance != wifiInstance->mUart.Instance){
        return;
    }
    __HAL_RCC_USART3_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    HAL_NVIC_SetPriority(USART3_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USART3_IRQn);
}

void ESP8266::uartDeinitCallback(UART_HandleTypeDef *huart)
{
    __HAL_RCC_USART3_CLK_DISABLE();
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_10 | GPIO_PIN_11);
    HAL_NVIC_DisableIRQ(USART3_IRQn);
}

bool ESP8266::isConnected() {
    if (mMode != Station && mMode != StationAndSoftAP) {
        return false;
    }

    sendCommand("AT+CIPSTATUS", true);

    const uint32_t timeout = 3000 + HAL_GetTick();
    const char *statusStr = "STATUS:";
    const uint32_t statusStrSize = std::strlen(statusStr);
    std::optional<uint32_t> posOpt = 0;

    while ((HAL_GetTick() < timeout)) {
        posOpt = mBuffer.find(statusStr);

        if (posOpt && mBuffer.size() > statusStrSize) {
            break;
        }
    }

    if (!posOpt) {
        return false;
    }

    const uint32_t pos = *posOpt + std::strlen(statusStr);

    if (pos >=  mBuffer.size()) {
        return false;
    }

    char statusChar = mBuffer[pos];

    HAL_Delay(500);

    bool connected = statusChar == '2' || statusChar == '3';
    if (!connected) {
        LOG(mBuffer.c_str());
    }

    return connected;
}
