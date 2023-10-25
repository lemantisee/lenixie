#pragma once

#include "stm32f1xx.h"
#include <cstdint>

class Logger {
public:
    static void init(USART_TypeDef *usart, uint32_t baudrate);
    static void log(const char *str);
    static void uartInterrupt();
private:
    static void uartReceiveCallback(UART_HandleTypeDef *uart, uint16_t size);
    static void uartReceiveCallback2(UART_HandleTypeDef *uart);
    static UART_HandleTypeDef mUsart;

};
