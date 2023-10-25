#include "DynamicIndication.h"

#include "Logger.h"
#include "SString.h"

void DynamicIndication::setDecoderPins(GPIO_TypeDef *port, uint16_t Apin, uint16_t Bpin, uint16_t Cpin, uint16_t Dpin)
{
    mDecoder.init(port, Apin, Bpin, Cpin, Dpin);
    mTimer = 0;
}

void DynamicIndication::setSign(Tube tube, GPIO_TypeDef *port, uint16_t pin)
{
    if (tube >= mSigns.size())
    {
        return;
    }

    mSigns[tube] = {.port = port, .pin = pin, .number = 0};

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(port, &GPIO_InitStruct);

    HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET);
}

void DynamicIndication::process()
{
    if (const Sign *sign = getCurrentSign())
    {
        clearSigns();
        HAL_GPIO_WritePin(sign->port, sign->pin, GPIO_PIN_SET);
        mDecoder.setValue(sign->number);
    }
}

void DynamicIndication::setNumber(uint8_t number1, uint8_t number2, uint8_t number3, uint8_t number4)
{
    mSigns[0].number = number1;
    mSigns[1].number = number2;
    mSigns[2].number = number3;
    mSigns[3].number = number4;
}

void DynamicIndication::clearSigns()
{
    for (uint8_t i = 0; i < 4; i++)
    {
        HAL_GPIO_WritePin(mSigns[i].port, mSigns[i].pin, GPIO_PIN_RESET);
    }

    for (int i = 0; i < 4000; i++)
    {
        asm("nop");
    }
}

const DynamicIndication::Sign *DynamicIndication::getCurrentSign()
{
    ++mTimer;
    const uint8_t indicationPeriod = 4;

    for (uint8_t i = 0; i < mSigns.size(); ++i)
    {
        if (mTimer == (i * indicationPeriod + 1))
        {
            return &mSigns[i];
        }
    }

    if (mTimer >= (indicationPeriod * mSigns.size() + 1))
    {
        mTimer = 0;
    }

    return nullptr;
}
