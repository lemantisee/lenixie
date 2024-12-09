#pragma once

#include <array>

#include "BCDDecoder.h"

class DynamicIndication
{
public:
    enum Tube {
        MSBHourTube = 0,
        LSBHourTube = 1,
        MSBMinutesTube = 2,
        LSBMinutesTube = 3,
    };

    DynamicIndication();

    void setDecoderPins(GPIO_TypeDef *port, uint16_t Apin, uint16_t Bpin, uint16_t Cpin,
                        uint16_t Dpin);
    void setSign(Tube tube, GPIO_TypeDef *port, uint16_t pin);
    void setNumber(Tube tube, uint8_t number);
    void start();
    void process();
    void dimm(bool state);

private:
    enum VisualStage { FullBrightness, FadeIn, FadeOut, Dimmed };

    struct Sign
    {
        Sign() = default;
        Sign(GPIO_TypeDef *port, uint16_t pin);

        uint8_t number = 0;

        void turnOff();
        void turnOn();

    private:
        GPIO_TypeDef *mPort = nullptr;
        uint16_t mPin = 0;
    };

    void processFade();

    uint8_t mCurrentSignIndex = 0;

    BCDDecoder mDecoder;
    std::array<Sign, 4> mSigns;
    uint32_t mTimerUs = 0;
    VisualStage mVisualStage = FullBrightness;
    uint32_t mSingOnUs = 0;
    uint32_t mFadeTimerUs = 0;
    bool mStarted = false;
};
