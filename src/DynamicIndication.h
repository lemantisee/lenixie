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

	void setDecoderPins(uint32_t port, uint16_t Apin, uint16_t Bpin, uint16_t Cpin, uint16_t Dpin);
	void setSign(Tube tube, uint32_t port, uint16_t pin);
	void setNumber(uint8_t number1, uint8_t number2, uint8_t number3, uint8_t number4);
	void process();
	
private:
	struct Sign
	{
		uint32_t port = 0;
		uint16_t pin = 0;
		uint8_t number = 0;
	};
	void clearSigns();
	std::optional<Sign> getCurrentSign();
	
	BCDDecoder mDecoder;
	std::array<Sign, 4> mSigns;
	uint8_t mCurrentSignsNumber = 0;
	uint8_t mCurrentSigns = 0;
	volatile uint8_t mTimer = 0;
};

