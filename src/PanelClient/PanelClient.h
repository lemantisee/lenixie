#pragma once

#include "UsbDevice.h"

#include "LogSession.h"

class PanelClient
{
public:
    PanelClient();
    
    bool init();
    void process();

private:
    UsbDevice mUsb;
    LogSession mLog;
};