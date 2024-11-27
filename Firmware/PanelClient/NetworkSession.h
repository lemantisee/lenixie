#pragma once

#include "MessageSession.h"

class ESP8266;

class NetworkSession : public MessageSession
{
public:
    void handle(const PanelMessage &msg) override;
    void setWifi(ESP8266 *wifi);

private:
    void onState();
    void onConnect(const PanelMessage &msg);
    ESP8266 *mWifi = nullptr;
};