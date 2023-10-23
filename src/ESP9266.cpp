#include "ESP9266.h"

#include <cstring>

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/cm3/nvic.h>
// #include "Logger.h"

void ESP9266::init(uint32_t usart, uint32_t port, uint16_t rxPin, uint16_t txPin, uint32_t baudrate)
{
    answerReady = false;
    mTimeout = false;
    mMode = STAMode;
    mTimer = 0;
    mUart = usart;
    gpio_set_mode(port, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, txPin);
    gpio_set_mode(port, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_INPUT_FLOAT, rxPin);
    usart_set_baudrate(usart, baudrate);
    usart_set_databits(usart, 8);
    usart_set_stopbits(usart, USART_STOPBITS_1);
    usart_set_parity(usart, USART_PARITY_NONE);
    usart_set_flow_control(usart, USART_FLOWCONTROL_NONE);
    usart_set_mode(usart, USART_MODE_TX_RX);
    usart_enable_rx_interrupt(usart);

    usart_enable(usart);
    // GPIO_InitTypeDef GPIO_InitStructure;
    // GPIO_InitStructure.GPIO_Pin = txPin;
    // GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    // GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    // GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // GPIO_InitStructure.GPIO_Pin = rxPin;
    // GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    // GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    // GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // espUart = usart;
    // uart.USART_BaudRate = baudrate;
    // uart.USART_WordLength = USART_WordLength_8b;
    // uart.USART_StopBits = USART_StopBits_1;
    // uart.USART_Parity = USART_Parity_No;
    // uart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    // uart.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    // USART_Init(espUart, &uart);
    // USART_ITConfig(espUart, USART_IT_RXNE, ENABLE);
    // USART_Cmd(espUart, ENABLE);
    
    nvic_set_priority(NVIC_USART1_IRQ, 0x01);
    nvic_enable_irq(NVIC_USART1_IRQ);
    
    // NVIC_InitTypeDef NVIC_InitStructure;
    // NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    // NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;
    // NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    // NVIC_Init(&NVIC_InitStructure);
    // USART_ClearFlag(espUart, USART_FLAG_RXNE);
    clearBuffer();

    wait(500);
    reset();
    sendCommand("ATE0", true); //echo off
    waitForAnswer("OK", 1000);
    sendCommand("AT+CIPMUX=0", true);
    waitForAnswer("OK", 2000);
}

void ESP9266::sendCommand(const char *cmd, bool sendEnd) {
    // USART_ClearFlag(espUart, USART_FLAG_RXNE);
    //USART_ReceiveData(espUart);

    // const char* cmd1 = cmd;
    while (*cmd != '\0') {
        usart_send_blocking(mUart, *cmd);
        // while (!USART_GetFlagStatus(mUart, USART_FLAG_TXE));
        // USART_SendData(mUart, *cmd);
        ++cmd;
    }
    // Logger::instance().log(cmd1);
    if (sendEnd) {
        endCommand();
    }
}

void ESP9266::processUART()
{
    if (!usart_get_flag(mUart, USART_SR_RXNE)) {
        return;
    }

    uint8_t data = usart_recv_blocking(mUart);
    if (mCurrentByte < mBuffer.size()) {
        mBuffer[mCurrentByte] = data;
        mCurrentByte++;
        // Logger::instance().log(data);
    } else {
        clearBuffer();
    }

    // if (USART_GetFlagStatus(mUart, USART_FLAG_RXNE) != RESET) {
    //     USART_ClearFlag(mUart, USART_FLAG_RXNE);
    //     uint8_t data = USART_ReceiveData(mUart);
    //     if (currentByte < bufferSize){
    //         buffer[currentByte] = data;
    //         currentByte++;
    //         Logger::instance().log(data);
    //     }
    //     else {
    //         clearBuffer();
    //     }
            
    // }
}

bool ESP9266::setMode(espMode_t mode) {
    if (mMode == mode) {
        return true;
    }

    mMode = mode;
    switch (mMode)
    {
    case STAMode:
        // Logger::instance().log("Sending to ESP AT+CWMODE=1\n");
        sendCommand("AT+CWMODE=1", true);
        break;
    case SoftAP:
        // Logger::instance().log("Sending to ESP AT+CWMODE=2\n");
        sendCommand("AT+CWMODE=2", true);
        break;
    case STA_AP:
        // Logger::instance().log("Sending to ESP AT+CWMODE=3\n");
        sendCommand("AT+CWMODE=3", true);
        break;
    default:
        break;
    }
    
    return waitForAnswer("OK", 5000);
}

bool ESP9266::waitForAnswer(const char *answer1, uint16_t timeout, const char *answer2) {
    mTimer = 0;
    while ((mTimer < timeout)) {
        if (answer2 != nullptr){
            if ((strstr((char *)mBuffer.data(), answer1) != nullptr) || (strstr((char *)mBuffer.data(), answer2) != nullptr)) {
                //Logger::instance().log((char *)buffer);
                return true;
            }
        }
        else{
            if (strstr((char *)mBuffer.data(), answer1) != nullptr) {
                //Logger::instance().log((char *)buffer);
                return true;
            }
        }
        asm("nop");
        //for (int i = 0; i < 20000; i++) asm("nop");

    }
    //Logger::instance().log("waitForAnswer failed\n");
    clearBuffer();
    return false;
}

void ESP9266::processTimer() {
    mTimer++;
}

bool ESP9266::connectNetwork(const char* ssid, const char* password) {
    setMode(ESP9266::STAMode);
    // Logger::instance().log("Connecting to wifi network\n");
    sendCommand("AT+CWJAP=");
    sendCommand("\"");
    sendCommand(ssid);
    sendCommand("\"");
    sendCommand(",");
    sendCommand("\"");
    sendCommand(password);
    sendCommand("\"", true);
    bool result = waitForAnswer("OK", 20000);
    // if (result) Logger::instance().log("Connected to wifi network\n");
    // else Logger::instance().log("Not connected to wifi network\n");
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
    usart_send_blocking(mUart, '\r');
    usart_send_blocking(mUart, '\n');
    clearBuffer();

    // while (!USART_GetFlagStatus(mUart, USART_FLAG_TXE));
    // USART_SendData(mUart, '\r');
    // while (!USART_GetFlagStatus(mUart, USART_FLAG_TXE));
    // USART_SendData(mUart, '\n');
    // while (!USART_GetFlagStatus(mUart, USART_FLAG_TXE));
    // clearBuffer();
    // Logger::instance().log("\r\n");
}

bool ESP9266::test() {
    // USART_ClearFlag(mUart, USART_FLAG_RXNE);
    clearBuffer();
    sendCommand("AT", true);
    // Logger::instance().log("Send command AT");
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
    clearBuffer();
    sendCommand("AT+CIPSEND=48", true);
    waitForAnswer(">", 1000);
    clearBuffer();
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
    for (uint16_t i = 0; i < size; i++) {
        usart_send_blocking(mUart, data[i]);
        // while (!USART_GetFlagStatus(mUart, USART_FLAG_TXE));
        // USART_SendData(mUart, data[i]);
    }

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
    //clearBuffer();
    char buffSize[4];
    char cmdStr[8] = "+IPD,";
    memset(buffSize, 0, 4);
    sprintf(buffSize, "%i", size);
    strcat(cmdStr, buffSize);
    if (waitForAnswer(cmdStr, 2000)){
        wait(1000);
        char *ptr = strstr((char *)mBuffer.data(), cmdStr);
        ///while (currentByte < (size + 10));
        return ((uint8_t*)ptr + 8);
    }
    return nullptr;
}

void ESP9266::switchToAP()
{
    // Logger::instance().log("Starting AP\n");
	clearBuffer();
    setMode(ESP9266::SoftAP);
	clearBuffer();
	setAP("NxC2", "NxCPass12", "192.168.4.1");
	clearBuffer();
    connectToServerUDP("192.168.4.255", 1111);
	clearBuffer();
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
    clearBuffer();
    wait(1000);
    uint8_t strSize = strlen(str);
    char buffSize[4];
    memset(buffSize, 0, 4);
    snprintf(buffSize, 4, "%d", strSize);

    sendCommand("AT+CIPSEND=");
    sendCommand(buffSize, true);
    waitForAnswer(">", 1000);
    // Logger::instance().log("Sending data:");
    // Logger::instance().log(str);
    // Logger::instance().log("\n");
    sendCommand(str);
    waitForAnswer("SEND OK", 2000);
}

void ESP9266::SendString(const char *str, const char *ip, uint16_t port) {
    clearBuffer();
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
    // Logger::instance().log("Sending data:");
    // Logger::instance().log(str);
    // Logger::instance().log("\n");
    sendCommand(str);
    waitForAnswer("SEND OK", 2000);
}

uint8_t ESP9266::getBufferSize() {
    return mBuffer.size();
}

uint8_t * ESP9266::getIncomeData() {
    char *ptr = strchr((char *)mBuffer.data(), ':');
    return ((uint8_t*)ptr + 1);
}

bool ESP9266::hasIncomeData() {
    if (strstr((char *)mBuffer.data(), "+IPD") != nullptr) {
        mTimer = 0;
        while (mTimer < 500);
        return true;
    }
    return false;
}

void ESP9266::clearBuffer() {
    mCurrentByte = 0;
    mBuffer.fill(0);
}

void ESP9266::closeCurrentConnection() {
    // Logger::instance().log("Closing current connection\n");
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
    wait(5000);
    //waitForAnswer("OK", 10000);
    // Logger::instance().log("Wifi reset complete\n");
}

void ESP9266::wait(uint16_t timeout) {
    mTimer = 0;
    while (mTimer < timeout){
        asm("nop");
    }
}

bool ESP9266::isConnected() {
    sendCommand("AT+CIPSTATUS", true);
    return waitForAnswer("STATUS:2", 3000, "STATUS:3");
}
