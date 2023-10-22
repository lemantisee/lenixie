#include "DynamicIndication.h"

#include <libopencm3/stm32/gpio.h>

void DynamicIndication::setSign(Tube tube, uint32_t port, uint16_t pin)
{
    if (tube >= mSigns.size()) {
        return;
    }

    mSigns[tube] = {.port = port, .pin = pin, .number = 0};

    gpio_set_mode(port, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, pin);
    currentSignsNumber++;
    start = false;
}

void DynamicIndication::setNumbersPin(BCDDecoder *decoder)
{
    mDecoder = decoder;
    currentSignsNumber = 0;
    mTimer = 0;
}

void DynamicIndication::process()
{
    if(!start) {
        return;
    }

    if(auto sign = getCurrentSign()) {
        clearSigns();
        gpio_set(sign->port, sign->pin);
        mDecoder->setValue(sign->number);
    }
}

void DynamicIndication::setNumber(Tube tube, uint8_t number)
{
    if (tube < mSigns.size()) {
        mSigns[tube].number = number;
    }
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

void DynamicIndication::startIndication(bool state)
{
    start = state;
    clearSigns();
}
