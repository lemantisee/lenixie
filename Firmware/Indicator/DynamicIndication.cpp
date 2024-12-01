#include "DynamicIndication.h"

namespace {
constexpr uint8_t updateFreqUs = 50;
constexpr uint16_t indicationFrameTicks = (16 * 1000) / updateFreqUs; // 16ms - 320
constexpr uint16_t fullBrightnessTicks = (3 * 1000) / updateFreqUs;   // 3 ms
constexpr uint8_t dimmedTicks = 200 / updateFreqUs; // 200us
constexpr uint32_t fadeTicks = (500 * 1000) / updateFreqUs; //500ms

constexpr uint32_t fadeTicksRange = fullBrightnessTicks - dimmedTicks;
constexpr uint32_t fadeTicksStep = fadeTicks / fadeTicksRange;
} // namespace

DynamicIndication::DynamicIndication()
{
    mSigns.back().isDummy = true;
    mSingOnTime = fullBrightnessTicks;
}

void DynamicIndication::setDecoderPins(GPIO_TypeDef *port, uint16_t Apin, uint16_t Bpin,
                                       uint16_t Cpin, uint16_t Dpin)
{
    mDecoder.init(port, Apin, Bpin, Cpin, Dpin);
    mTimer = 0;
}

void DynamicIndication::setSign(Tube tube, GPIO_TypeDef *port, uint16_t pin)
{
    if (tube >= mSigns.size()) {
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
    // called every 50us

    updateDimm();

    if (const Sign *sign = getCurrentSign()) {
        clearSigns();
        if (sign->port && !sign->isDummy) {
            HAL_GPIO_WritePin(sign->port, sign->pin, GPIO_PIN_SET);
            mDecoder.setValue(sign->number);
        }
    }
}

void DynamicIndication::dimm(bool state)
{
    if (state && mVisualStage == Dimmed) {
        return;
    }

    if (!state && mVisualStage == FullBrightness) {
        return;
    }

    mVisualStage = state ? FadeIn : FadeOut;
}

void DynamicIndication::setNumber(uint8_t number1, uint8_t number2, uint8_t number3,
                                  uint8_t number4)
{
    mSigns[0].number = number1;
    mSigns[1].number = number2;
    mSigns[2].number = number3;
    mSigns[3].number = number4;
}

void DynamicIndication::clearSigns()
{
    for (uint8_t i = 0; i < 4; i++) {
        const Sign &sign = mSigns[i];
        if (!sign.port) {
            continue;
        }
        HAL_GPIO_WritePin(sign.port, sign.pin, GPIO_PIN_RESET);
    }

    //TODO this function called in interrupt. Not allowed any delays here
    // for (int i = 0; i < 4000; i++) {
    //     asm("nop");
    // }
}

const DynamicIndication::Sign *DynamicIndication::getCurrentSign()
{
    ++mTimer;

    for (uint8_t i = 0; i < mSigns.size(); ++i) {
        if (mTimer == (i * mSingOnTime + 1)) {
            return &mSigns[i];
        }
    }

    if (mTimer >= indicationFrameTicks + 1) {
        mTimer = 0;
    }

    return nullptr;
}

void DynamicIndication::updateDimm()
{
    if (mVisualStage != FadeIn && mVisualStage != FadeOut) {
        return;
    }

    ++mFadeTicks;

    for (uint32_t r = 0; r < fadeTicksRange; ++r) {
        if (r * fadeTicksStep == mFadeTicks) {
            if (mVisualStage == FadeIn) {
                --mSingOnTime;
            } else if (mVisualStage == FadeOut) {
                ++mSingOnTime;
            }
            break;
        }
    }

    if (mFadeTicks != fadeTicks) {
        return;
    }

    mFadeTicks = 0;

    if (mVisualStage == FadeIn) {
        mSingOnTime = dimmedTicks;
        mVisualStage = Dimmed;
        return;
    }

    if (mVisualStage == FadeOut) {
        mSingOnTime = fullBrightnessTicks;
        mVisualStage = FullBrightness;
    }
}
