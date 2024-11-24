#pragma once

#include "MessageSession.h"

class LogSession : public MessageSession
{
public:
    void handle(const PanelMessage &msg) override;

private:
    void sendEnd();
    SString<256> escapeString(const SString<128> &str) const;
};
