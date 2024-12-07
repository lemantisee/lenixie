#include "DynamicIndication.h"

namespace {
constexpr uint8_t updateFreqUs = 50;
constexpr uint32_t indicationFrameUs = 16 * 1000;
constexpr uint32_t fullBrightnessUs = 3 * 1000;
constexpr uint32_t dimmedUs = 50;
constexpr uint32_t fadeTransitionUs = 500 * 1000;

constexpr uint32_t fadeRangeUs = fullBrightnessUs - dimmedUs;
constexpr uint32_t updateFadeStepUs = ((fadeTransitionUs / fadeRangeUs) / updateFreqUs + 1) * updateFreqUs;
} // namespace

DynamicIndication::DynamicIndication() : mSingOnUs(fullBrightnessUs)
{
    mSigns.back().isDummy = true;
}

void DynamicIndication::setDecoderPins(GPIO_TypeDef *port, uint16_t Apin, uint16_t Bpin,
                                       uint16_t Cpin, uint16_t Dpin)
{
    mDecoder.init(port, Apin, Bpin, Cpin, Dpin);
    mTimerUs = 0;
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

    processFade();

    mTimerUs += updateFreqUs;

    if (const Sign *sign = getCurrentSign()) {
        clearSigns();
        if (sign->port && !sign->isDummy) {
            HAL_GPIO_WritePin(sign->port, sign->pin, GPIO_PIN_SET);
            mDecoder.setValue(sign->number);
        }
    }

    if (mTimerUs > indicationFrameUs) {
        mTimerUs = 0;
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
}

const DynamicIndication::Sign *DynamicIndication::getCurrentSign() const
{
    for (uint8_t i = 0; i < mSigns.size(); ++i) {
        const uint32_t leftEdge = i * mSingOnUs;
        const uint32_t rifhtEdge = leftEdge + updateFreqUs;
        if (mTimerUs > leftEdge && mTimerUs <= rifhtEdge) {
            return &mSigns[i];
        }
    }

    return nullptr;
}

void DynamicIndication::processFade()
{
    if (mVisualStage != FadeIn && mVisualStage != FadeOut) {
        return;
    }

    mFadeTimerUs += updateFreqUs;

    const bool updateFade = (mFadeTimerUs % updateFadeStepUs) == 0;
    if (updateFade) {
        if (mVisualStage == FadeIn) {
            --mSingOnUs;
        } else if (mVisualStage == FadeOut) {
            ++mSingOnUs;
        }
    }

    if (mFadeTimerUs < fadeTransitionUs) {
        return;
    }

    mFadeTimerUs = 0;

    if (mVisualStage == FadeIn) {
        mVisualStage = Dimmed;
        mSingOnUs == dimmedUs;
        return;
    }

    if (mVisualStage == FadeOut) {
        mVisualStage = FullBrightness;
        mSingOnUs = fullBrightnessUs;
    }
}
