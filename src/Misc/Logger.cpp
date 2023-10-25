#include "Logger.h"

#include <cstring>
#include <array>

#include "StringBuffer.h"

UART_HandleTypeDef Logger::mUsart;

namespace
{
    std::array<uint8_t, 10> buffer = {};
} // namespace


void Logger::init(USART_TypeDef *usart, uint32_t baudrate)
{
    mUsart.Instance = usart;
    mUsart.Init.BaudRate = baudrate;
    mUsart.Init.WordLength = UART_WORDLENGTH_8B;
    mUsart.Init.StopBits = UART_STOPBITS_1;
    mUsart.Init.Parity = UART_PARITY_NONE;
    mUsart.Init.Mode = UART_MODE_TX_RX;
    mUsart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    mUsart.Init.OverSampling = UART_OVERSAMPLING_16;
    HAL_UART_Init(&mUsart);
    HAL_UART_RegisterRxEventCallback(&mUsart, Logger::uartReceiveCallback);
    HAL_UARTEx_ReceiveToIdle_IT(&mUsart, buffer.data(), buffer.size());
}

void Logger::uartReceiveCallback2(UART_HandleTypeDef *uart)
{
    log("receive2");
    HAL_UART_Transmit(&mUsart, buffer.data(), buffer.size(), 100);
}

void Logger::log(const char *str)
{
    HAL_UART_Transmit(&mUsart, (uint8_t *)str, std::strlen(str), 100);
    HAL_UART_Transmit(&mUsart, (uint8_t *)"\n", 1, 100);
}

void Logger::uartInterrupt()
 {
    HAL_UART_IRQHandler(&mUsart);
}

void Logger::uartReceiveCallback(UART_HandleTypeDef *uart, uint16_t size)
{
    if(uart != &mUsart) {
        return;
    }

    HAL_UART_Transmit(&mUsart, buffer.data(), size, 1000);

    buffer.fill(0);
    HAL_UARTEx_ReceiveToIdle_IT(&mUsart, buffer.data(), buffer.size());
}
