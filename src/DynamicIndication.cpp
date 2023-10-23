#include "DynamicIndication.h"

#include <libopencm3/stm32/gpio.h>

void DynamicIndication::setDecoderPins(uint32_t port, uint16_t Apin, uint16_t Bpin, uint16_t Cpin, uint16_t Dpin)
{
    mDecoder.init(port, Apin, Bpin, Cpin, Dpin);
    mCurrentSignsNumber = 0;
    mTimer = 0;
}

void DynamicIndication::setSign(Tube tube, uint32_t port, uint16_t pin)
{
    if (tube >= mSigns.size()) {
        return;
    }

    mSigns[tube] = {.port = port, .pin = pin, .number = 0};

    gpio_set_mode(port, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, pin);
    mCurrentSignsNumber++;
}

void DynamicIndication::process()
{
    if (auto sign = getCurrentSign()) {
        clearSigns();
        gpio_set(sign->port, sign->pin);
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
    for (uint8_t i = 0; i < 4; i++) {
        gpio_clear(mSigns[i].port, mSigns[i].pin);
    }

    for (int i = 0; i < 4000; i++) {
        asm("nop");
    }
}

std::optional<DynamicIndication::Sign> DynamicIndication::getCurrentSign()
{
    const uint8_t indicationPeriod = 4;

    for (uint8_t i = 0; i < mSigns.size(); ++i) {
        if (mTimer == i * indicationPeriod) {
            return mSigns[i];
        }
    }

    // if (internalTimer == 0) {
    //     return mSigns[0];
    // }

    // if (internalTimer == 4) {
    //     return mSigns[1];
    // }

    // if (internalTimer == 8) {
    //     return mSigns[2];
    // }

    // if (internalTimer == 12) {
    //     return mSigns[3];
    // }

    mTimer++;

    if (mTimer >= indicationPeriod * mSigns.size()) {
        mTimer = 0;
    }

    return std::nullopt;
}
