#include "PanelClient.h"

PanelClient::PanelClient() 
{
    mLog.setClient(&mUsb);
}

bool PanelClient::init() { return mUsb.init(); }

void PanelClient::process()
{
    const SString<64> inBuffer = mUsb.popData();
    if (inBuffer.empty()) {
        return;
    }

    const PanelMessage msg = PanelMessage::fromReport(inBuffer);

    mLog.handle(msg);
}
