#pragma once

#include <array>
#include <optional>

#include "BCDDecoder.h"

class DynamicIndication
{
public:
    enum Tube
    {
        MSBHourTube    = 0,
        LSBHourTube    = 1,
        MSBMinutesTube = 2,
        LSBMinutesTube = 3     
    };

	DynamicIndication () = default;
	
	void setSign(Tube tube, uint32_t port, uint16_t pin);
	void setNumbersPin(BCDDecoder *decoder);
	void setNumber(Tube tube, uint8_t number);
	void process();
	void startIndication(bool state);
	
private:
	struct Sign
	{
		uint32_t port = 0;
		uint16_t pin = 0;
		uint8_t number = 0;
	};
	void clearSigns();
	std::optional<Sign> getCurrentSign();
	
	BCDDecoder *mDecoder = nullptr;
	std::array<Sign, 4> mSigns;
	uint8_t currentSignsNumber = 0;
	uint8_t currentSigns = 0;
	volatile uint8_t mTimer = 0;
	volatile bool start = false;
	

};

