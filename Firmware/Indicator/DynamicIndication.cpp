#include "DynamicIndication.h"

namespace {
constexpr uint8_t updateFreqUs = 50;
constexpr uint32_t indicationFrameUs = 16 * 1000;
constexpr uint32_t indicationSignUs = indicationFrameUs / 4;
constexpr uint32_t fullBrightnessUs = 3 * 1000;
constexpr uint32_t dimmedUs = 50;
constexpr uint32_t fadeTransitionUs = 500 * 1000;

constexpr uint32_t fadeRangeUs = fullBrightnessUs - dimmedUs;
constexpr uint32_t updateFadeStepUs = ((fadeTransitionUs / fadeRangeUs) / updateFreqUs + 1) * updateFreqUs;
} // namespace

DynamicIndication::DynamicIndication() : mSingOnUs(fullBrightnessUs) {}

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

    mSigns[tube] = Sign(port, pin);
}

void DynamicIndication::setNumber(Tube tube, uint8_t number) 
{
    if (tube >= mSigns.size()) {
        return;
    }



    mSigns[tube].number = std::min<uint8_t>(number, 9);
}

void DynamicIndication::start() 
{
    mStarted = true;
}

void DynamicIndication::process()
{
    if (!mStarted) {
        return;
    }
    // called every 50us

    if (mCurrentSignIndex >= mSigns.size()) {
        return;
    }

    processFade();

    mTimerUs += updateFreqUs;

    if (mTimerUs >= mSingOnUs) {
        mSigns[mCurrentSignIndex].turnOff();
    }

    if (mTimerUs >= indicationSignUs) {
        mTimerUs = 0;
        ++mCurrentSignIndex;
        if (mCurrentSignIndex >= mSigns.size()) {
            mCurrentSignIndex = 0;
        }
        mDecoder.setValue(mSigns[mCurrentSignIndex].number);
        mSigns[mCurrentSignIndex].turnOn();
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

DynamicIndication::Sign::Sign(GPIO_TypeDef *port, uint16_t pin) : mPort(port), mPin(pin)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = mPin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(mPort, &GPIO_InitStruct);

    HAL_GPIO_WritePin(mPort, mPin, GPIO_PIN_RESET);
}

void DynamicIndication::Sign::turnOff() { HAL_GPIO_WritePin(mPort, mPin, GPIO_PIN_RESET); }

void DynamicIndication::Sign::turnOn() { HAL_GPIO_WritePin(mPort, mPin, GPIO_PIN_SET); }
