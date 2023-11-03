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
    HAL_UART_RegisterCallback(&mUsart, HAL_UART_MSPINIT_CB_ID, Logger::uartInitCallback);
    HAL_UART_RegisterCallback(&mUsart, HAL_UART_MSPDEINIT_CB_ID, Logger::uartDeinitCallback);
    HAL_UARTEx_ReceiveToIdle_IT(&mUsart, buffer.data(), buffer.size());
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

void Logger::uartInitCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance != mUsart.Instance){
        return;
    }

    __HAL_RCC_USART3_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    HAL_NVIC_SetPriority(USART3_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USART3_IRQn);
}

void Logger::uartDeinitCallback(UART_HandleTypeDef *huart)
{
    __HAL_RCC_USART3_CLK_DISABLE();
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_10 | GPIO_PIN_11);
}
