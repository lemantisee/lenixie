#include "BCDDecoder.h"

void BCDDecoder::init(GPIO_TypeDef *port, uint16_t Apin, uint16_t Bpin, uint16_t Cpin, uint16_t Dpin)
{
    mPort = port;
    mApin = Apin;
    mBpin = Bpin;
    mCpin = Cpin;
    mDpin = Dpin;

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = Apin | Bpin | Cpin | Dpin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(port, &GPIO_InitStruct);

    HAL_GPIO_WritePin(mPort, mApin | mBpin | mCpin | mDpin, GPIO_PIN_RESET);
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
}
