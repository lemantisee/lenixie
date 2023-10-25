#ifndef INTERFACE_H
#define INTERFACE_H

#include <stdint.h>

class Interface
{
public:
    Interface() = default;
    // virtual ~Interface() = default;
    
    virtual bool SendString(const char *str) = 0;
    virtual bool hasIncomeData() = 0;
    virtual uint8_t *getIncomeData() = 0;
};

#endif

