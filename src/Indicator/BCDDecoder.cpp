#include "BCDDecoder.h"

void BCDDecoder::init(GPIO_TypeDef *port, uint16_t Apin, uint16_t Bpin, uint16_t Cpin, uint16_t Dpin)
{
    mPort = port;
    mApin = Apin;
    mBpin = Bpin;
    mCpin = Cpin;
    mDpin = Dpin;

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9, GPIO_PIN_RESET);

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = Apin | Bpin | Cpin | Dpin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(port, &GPIO_InitStruct);

    // gpio_set_mode(port, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, Apin);
    // gpio_set_mode(port, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, Bpin);
    // gpio_set_mode(port, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, Cpin);
    // gpio_set_mode(port, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, Dpin);
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

    HAL_GPIO_WritePin(mPort, mApin | mBpin | mCpin | mDpin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(mPort, pinStatus, GPIO_PIN_SET);
    // gpio_clear(mPort, mApin | mBpin | mCpin | mDpin);
    // gpio_set(mPort, pinStatus);
}

void BCDDecoder::delay() const
{
    for (int i = 0; i < 30000; i++){
        asm("nop");
    }
}
