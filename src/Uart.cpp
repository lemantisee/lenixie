#include "Uart.h"

#include "Logger.h"

namespace {
Uart *uartInstance = nullptr;
}

extern "C" {
    void USART3_IRQHandler() { uartInstance->interrupt(); }
}

bool Uart::init(USART_TypeDef *usart, uint32_t baudrate)
{
#if USE_HAL_UART_REGISTER_CALLBACKS != 1
#error "Error. USART callback not enabled"
#endif

    uartInstance = this;

    if (!setup(usart, baudrate)) {
        LOG("Unable to setup uart");
        return false;
    }

    if (!startRead()) {
        LOG("Unable to start uart");
        return false;
    }

    return true;
}

void Uart::interrupt() { HAL_UART_IRQHandler(&mHandle); }

void Uart::send(const uint8_t *data, uint16_t size, uint32_t timeout) 
{
    HAL_UART_Transmit(&mHandle, data, size, timeout);
}

void Uart::send(const char *str, uint32_t timeout) 
{
    HAL_UART_Transmit(&mHandle, (uint8_t *)str, std::strlen(str), timeout);
}

void Uart::onReceive(std::function<void(const SString<64> &data)> func)
{
    mReceiveCallback = std::move(func);
}

bool Uart::setup(USART_TypeDef *usart, uint32_t baudrate)
{
    mHandle = {0};
    mHandle.Instance = usart;
    mHandle.Init.BaudRate = baudrate;
    mHandle.Init.WordLength = UART_WORDLENGTH_8B;
    mHandle.Init.StopBits = UART_STOPBITS_1;
    mHandle.Init.Parity = UART_PARITY_NONE;
    mHandle.Init.Mode = UART_MODE_TX_RX;
    mHandle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    mHandle.Init.OverSampling = UART_OVERSAMPLING_16;

    if (HAL_UART_RegisterCallback(&mHandle, HAL_UART_MSPINIT_CB_ID, Uart::uartInitCallback) != HAL_OK ) {
        return false;
    }

    if (HAL_UART_RegisterCallback(&mHandle, HAL_UART_MSPDEINIT_CB_ID, Uart::uartDeinitCallback) != HAL_OK ) {
        return false;
    }

    if (HAL_UART_Init(&mHandle) != HAL_OK ) {
        return false;
    }

    if (HAL_UART_RegisterRxEventCallback(&mHandle, Uart::uartReceiveCallback) != HAL_OK ) {
        return false;
    }

    return true;
}

bool Uart::startRead()
{
    mInputBuffer.clear();
    return HAL_UARTEx_ReceiveToIdle_IT(&mHandle, (uint8_t *)mInputBuffer.data(), mInputBuffer.capacity()) == HAL_OK;
}

void Uart::uartReceiveCallback(UART_HandleTypeDef *uart, uint16_t size)
{
    if (uart != &uartInstance->mHandle) {
        return;
    }

    if (size > uartInstance->mInputBuffer.capacity()) {
        uartInstance->startRead();
        return;
    }

    uartInstance->mInputBuffer.resize(size);

    if (uartInstance->mReceiveCallback) {
        uartInstance->mReceiveCallback(uartInstance->mInputBuffer);
    }

    uartInstance->startRead();
}

void Uart::uartInitCallback(UART_HandleTypeDef *huart)
{
    if (!uartInstance) {
        return;
    }

    if (huart->Instance != uartInstance->mHandle.Instance){
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

void Uart::uartDeinitCallback(UART_HandleTypeDef *huart)
{
    if (!uartInstance) {
        return;
    }

    if (huart->Instance != uartInstance->mHandle.Instance){
        return;
    }

    __HAL_RCC_USART3_CLK_DISABLE();
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_10 | GPIO_PIN_11);
    HAL_NVIC_DisableIRQ(USART3_IRQn);
}
