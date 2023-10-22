#include "BCDDecoder.h"

#include <libopencm3/stm32/gpio.h>

void BCDDecoder::init(uint32_t port, uint16_t Apin, uint16_t Bpin, uint16_t Cpin, uint16_t Dpin)
{
    mPort = port;
    mApin = Apin;
    mBpin = Bpin;
    mCpin = Cpin;
    mDpin = Dpin;

    gpio_set_mode(port, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, Apin);
    gpio_set_mode(port, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, Bpin);
    gpio_set_mode(port, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, Cpin);
    gpio_set_mode(port, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, Dpin);
}

void BCDDecoder::setValue(uint8_t value)
{
    
    uint16_t pinStatus = 0;

    if ((value & (1 << 0)))
    {
        pinStatus |= mApin;
    }
    if ((value & (1 << 1))) {
        pinStatus |= mBpin;
    }
    if ((value & (1 << 2))) {
        pinStatus |= mCpin;
    }
    if ((value & (1 << 3))) {
        pinStatus |= mDpin;
    }

    gpio_clear(mPort, mApin | mBpin | mCpin | mDpin);
    gpio_set(mPort, pinStatus);
}

void BCDDecoder::delay() const
{
    for (int i = 0; i < 30000; i++){
        asm("nop");
    }
}
