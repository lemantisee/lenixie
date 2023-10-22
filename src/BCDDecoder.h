#pragma once

#include <stdint.h>

class BCDDecoder
{
public:
	BCDDecoder() = default;
	void init(uint32_t port, uint16_t Apin, uint16_t Bpin, uint16_t Cpin, uint16_t Dpin);
	void setValue(uint8_t value);
	
private:
	void delay() const;

	uint32_t mPort = 0;
	uint16_t mApin = 0; 
	uint16_t mBpin = 0;
	uint16_t mCpin = 0; 
	uint16_t mDpin = 0;
};

