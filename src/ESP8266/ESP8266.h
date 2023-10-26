#pragma once

#include "SMProtocol/Interface.h"

#include "stm32f1xx.h"
#include "StringBuffer.h"

class EspAtCommand;

class ESP8266 : public Interface
{
public:
	enum  Mode
	{
         Unknown = 0, Station = 1, SoftAP = 2, StationAndSoftAP = 3
	};

	ESP8266() = default;

	void init(USART_TypeDef *usart, uint32_t baudrate);

	bool isConnected();
	bool connectNetwork(const char* ssid, const char* password);

	bool setAPip(const char *ip);

	bool switchToAP();
	
	bool connectToServerUDP(const char* host, uint16_t port);
	bool sendUDPpacket(const char* msg, uint16_t size);

    bool SendString(const char *str) override;
    bool SendString(const char *str, const char *ip, uint16_t port);
    bool hasIncomeData() override;
    uint8_t *getIncomeData() override;
	bool getData(uint8_t *buffer, uint8_t size);

	void uartInterrupt();
	
	char broadcastIP[17];

private:
	enum Encryption {
		Open = 0,
		WPA_PSK = 2,
		WPA2_PSK = 3,
		WPA_WPA2_PSK = 4,
	};

	void startReadUart();
	bool setMode(Mode mode);
	bool setAP(const char *ssid, const char *pass);
	void sendCommand(const char *cmd, bool sendEnd = false);
	void sendCommand(const EspAtCommand &cmd);
	bool waitForAnswer(const char *answer1, uint16_t timeout, const char *answer2 = nullptr);
	bool test();
	void reset();
	void clearBuffer();
	bool getIP();
	void closeCurrentConnection();
	
	static void uartReceiveCallback(UART_HandleTypeDef *uart, uint16_t size);
	
	UART_HandleTypeDef mUart;
	StringBuffer<255> mBuffer;
	StringBuffer<64> mInputBuffer;
	bool answerReady = false;
	Mode mMode = Unknown;
	bool mTimeout = false;
	char ipoctet1[4];
	char ipoctet2[4];
	char ipoctet3[4];
	char ipoctet4[4];
	
};

