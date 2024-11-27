#include "PanelClient.h"

PanelClient::PanelClient() 
{
    mLog.setClient(&mUsb);
    mDateTime.setClient(&mUsb);
    mNetwork.setClient(&mUsb);

    mLog.setNext(&mDateTime);
    mDateTime.setNext(&mNetwork);
}

bool PanelClient::init(RTClock *clock, ESP8266 *wifi) 
{
    mDateTime.setRtc(clock);
    mNetwork.setWifi(wifi);
    
    return mUsb.init(); 
}

void PanelClient::process()
{
    mUsb.process();
    
    const SString<256> inBuffer = mUsb.popData();
    if (inBuffer.empty()) {
        return;
    }

    const PanelMessage msg = PanelMessage::fromReport(inBuffer);

    mLog.handle(msg);
}
