#pragma once

#include <functional>

#include "stm32f1xx.h"

#include "SString.h"

class Uart
{
public:
    Uart() = default;
    bool init(USART_TypeDef *usart, uint32_t baudrate);
    void interrupt();

    bool startRead();

    void send(const uint8_t *data, uint16_t size, uint32_t timeout);
    void send(const char *str, uint32_t timeout);
    void onReceive(std::function<void(const SString<64> &data)> func);

private:
    static void uartReceiveCallback(UART_HandleTypeDef *uart, uint16_t size);
    static void uartInitCallback(UART_HandleTypeDef *huart);
    static void uartDeinitCallback(UART_HandleTypeDef *huart);

    bool setup(USART_TypeDef *usart, uint32_t baudrate);

    SString<64> mInputBuffer;
    UART_HandleTypeDef mHandle;
    std::function<void(const SString<64> &data)> mReceiveCallback;
};
