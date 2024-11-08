#ifndef SMPROTOCOL_H
#define SMPROTOCOL_H

#include <cstdlib>
#include <cstring>
#include <cstring>
#include <cstdio>
#include <cstdint>
#include <array>

class Interface;

class SMProtocol
{
public:
	SMProtocol() = default;
	
	void init(Interface *inter);
	bool isCommandReady();
	uint8_t getCommand();
	uint16_t getParametr();
	uint8_t *getStringParametr();
	void SendCommand(const char *str);
	void SendCommand(const char *str, uint32_t parametr);
	void SendCommand(const char *str, const char *parametr);
	void SendCommand(uint8_t cmd, uint32_t parametr);
	void SendCommand(uint8_t cmd, const char *parametr);
	
	
private:
	Interface *mInterface = nullptr;
	uint16_t mParametr = 0;
	std::array<char, 100> mStringParametr = {};
};

#endif

