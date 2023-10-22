#pragma once

#include "SMProtocol/Interface.h"

#include <array>

class ESP9266 : public Interface
{
public:
	enum  espMode_t
	{
         STAMode = 1, SoftAP = 2, STA_AP = 3
	};

	ESP9266() = default;

	void init(uint32_t usart, uint32_t port, uint16_t rxPin, uint16_t txPin, uint32_t baudrate);
	void sendCommand(const char *cmd, bool sendEnd = false);
	void sendData(uint8_t *data, uint16_t size);
	bool getIP();
	void processUART();
	void processTimer();
	void clearBuffer();
	void closeCurrentConnection();
	bool isConnected();
	bool connectNetwork(const char* ssid, const char* password);
	bool connectToServerUDP(const char* host, uint16_t port);
	bool sendUDPpacket(const char* msg, uint16_t size);
	bool sendUDPpacket(const char* msg, const char* size);
	bool setAPip(const char *ip);
	void reset();
	uint8_t *getData(uint8_t size);
	void switchToAP();
	
    void SendString(const char *str) override;
    void SendString(const char *str, const char *ip, uint16_t port);
    bool hasIncomeData() override;
    uint8_t getBufferSize() override;
    uint8_t *getIncomeData() override;
	
	bool test();
	char broadcastIP[17];

private:
	bool setMode(espMode_t mode);
	bool setAP(const char *ssid, const char *pass, const char *ip);
	bool waitForAnswer(const char *answer1, uint16_t timeout, const char *answer2 = nullptr);
	void endCommand();
	void wait(uint16_t timeout);
	
	uint32_t mUart = 0;
	std::array<uint8_t, 255> mBuffer;
	uint16_t mCurrentByte = 0;
	bool answerReady = false;
	espMode_t mMode = STAMode;
    volatile uint16_t mTimer = 0;
	bool mTimeout = false;
	char ipoctet1[4];
	char ipoctet2[4];
	char ipoctet3[4];
	char ipoctet4[4];
	
};

