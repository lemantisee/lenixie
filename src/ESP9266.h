#pragma once

#include "SMProtocol/Interface.h"

#include "stm32f1xx.h"
#include "StringBuffer.h"

class ESP9266 : public Interface
{
public:
	enum  espMode_t
	{
         STAMode = 1, SoftAP = 2, STA_AP = 3
	};

	ESP9266() = default;

	void init(USART_TypeDef *usart, uint32_t baudrate);
	bool getIP();

	void clearBuffer();
	bool isConnected();
	void closeCurrentConnection();
	bool connectToServerUDP(const char* host, uint16_t port);
	bool connectNetwork(const char* ssid, const char* password);

	bool setAPip(const char *ip);
	void reset();
	uint8_t *getData(uint8_t size);
	void switchToAP();
	
	bool sendUDPpacket(const char* msg, uint16_t size);
	bool sendUDPpacket(const char* msg, const char* size);

    void SendString(const char *str) override;
    void SendString(const char *str, const char *ip, uint16_t port);
    bool hasIncomeData() override;
    uint8_t *getIncomeData() override;

	void uartInterrupt();
	
	bool test();
	char broadcastIP[17];

private:
	void startReadUart();
	bool setMode(espMode_t mode);
	bool setAP(const char *ssid, const char *pass, const char *ip);
	void sendCommand(const char *cmd, bool sendEnd = false);
	void sendData(uint8_t *data, uint16_t size);
	bool waitForAnswer(const char *answer1, uint16_t timeout, const char *answer2 = nullptr);
	void endCommand();
	static void uartReceiveCallback(UART_HandleTypeDef *uart, uint16_t size);
	
	UART_HandleTypeDef mUart;
	StringBuffer<255> mBuffer;
	StringBuffer<64> mInputBuffer;
	bool answerReady = false;
	espMode_t mMode = STAMode;
	bool mTimeout = false;
	char ipoctet1[4];
	char ipoctet2[4];
	char ipoctet3[4];
	char ipoctet4[4];
	
};

