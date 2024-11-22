#include "PanelClient.h"

PanelClient::PanelClient() 
{
    mLog.setClient(&mUsb);
    mDateTime.setClient(&mUsb);

    mLog.setNext(&mDateTime);
}

bool PanelClient::init(const RTClock *clock) 
{
    mDateTime.setRtc(clock);
    
    return mUsb.init(); 
}

void PanelClient::process()
{
    const SString<64> inBuffer = mUsb.popData();
    if (inBuffer.empty()) {
        return;
    }

    const PanelMessage msg = PanelMessage::fromReport(inBuffer);

    mLog.handle(msg);
}
