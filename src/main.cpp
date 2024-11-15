#include "stm32f1xx.h"

#include "DynamicIndication.h"
#include "RTClock.h"
#include "ESP8266.h"
#include "Logger.h"
#include "SString.h"
#include "WifiCredentials.h"
#include "UsbDevice.h"
#include "JsonObject.h"
#include "MonitorCommand.h"
#include "LogDump.h"
#include "Uart.h"

namespace {
const uint32_t F_CPU = 72000000;

volatile bool testBlinkStart = false;
volatile uint8_t testTimer = 0;
uint16_t sysTicks = 0;

DynamicIndication Indication;
RTClock Clock;
ESP8266 wifi;
UsbDevice usbHost;
LogDump logDumper;
Uart uart;

enum Command {
    DeviceID = 100,
    SetWifi = 101,
    SetNtpServer = 102,
    SetTime = 103,
    ResetFabric = 104,
    TestBlink = 105,
    EnableNTP = 106,
    SetTimeZone = 107
};

bool systemClockInit()
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE | RCC_OSCILLATORTYPE_LSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.LSEState = RCC_LSE_ON;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        return false;
    }

    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1
                                  | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
        return false;
    }

    RCC_PeriphCLKInitTypeDef rtcClkInit = {0};
    rtcClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC;
    rtcClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
    if (HAL_RCCEx_PeriphCLKConfig(&rtcClkInit) != HAL_OK) {
        return false;
    }

    RCC_PeriphCLKInitTypeDef usbClkInit = {0};
    usbClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
    usbClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL_DIV1_5;
    if (HAL_RCCEx_PeriphCLKConfig(&usbClkInit) != HAL_OK) {
        return false;
    }

    HAL_SYSTICK_Config(F_CPU / 20000 - 1);

    return true;
}

void dumpLogs() {
    LogDump dumper;

    while (!dumper.empty()) {
        SString<64> msg = dumper.popReport();
        if (msg.empty()) {
            break;
        }
        usbHost.sendData(msg);

        SString<64> report;
        while (report.empty()) {
            report = usbHost.popData();
        }

        if (JsonObject(report).getInt("id", UnknownCommand) != GetLog) {
            break;
        }
    }

    JsonObject j;
    j.add("id", LogEnd);
    usbHost.sendData(j.dump());
}

void processUsbCmd(const SString<64> &buffer)
{
    JsonObject inMessage(buffer);

    MonitorCommandId id = MonitorCommandId(inMessage.getInt("id", UnknownCommand));

    switch (id) {
    case GetLog: dumpLogs(); break;
    default: break;
    }
}

void initWifi(ESP8266 &esp) 
{
    if (!uart.init(USART3, 115200)) {
        LOG("Unable to init uart");
        return;
    }

    if (!esp.init(&uart)) {
        LOG("Unable to init wifi");
        return;
    }

    if (esp.isConnected()) {
        return;
    }

    LOG("Connecting to wifi network %s", WifiCredentials::userSsid());
    if (!esp.connectNetwork(WifiCredentials::userSsid(), WifiCredentials::userPassword())) {
        LOG("Unable to connect to network");
    }
}

} // namespace

extern "C" {

void HAL_MspInit(void)
{
    __HAL_RCC_AFIO_CLK_ENABLE();
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
}

void SysTick_Handler()
{
    // called every 50us
    HAL_SYSTICK_IRQHandler();

    ++sysTicks;
    if (sysTicks == 20) {
        // called every 1ms
        sysTicks = 0;
        HAL_IncTick();
    }

    Indication.process();
}

void RTC_IRQHandler()
{
    Clock.interrupt();
    const DateTime &time = Clock.getTime();
    if (!time.isNull()) {
        Indication.setNumber(time.hours / 10, time.hours % 10, time.minutes / 10,
                             time.minutes % 10);
    }

    Indication.dimm(time.hours < 7);
}

}

int main(void)
{
    HAL_Init();
    systemClockInit();

    LOG("Started");

    if (!usbHost.init()) {
        return 1;
    }

    Indication.setDecoderPins(GPIOB, GPIO_PIN_6, GPIO_PIN_8, GPIO_PIN_9, GPIO_PIN_7);
    Indication.setSign(DynamicIndication::MSBHourTube, GPIOA, GPIO_PIN_6);
    Indication.setSign(DynamicIndication::LSBHourTube, GPIOA, GPIO_PIN_5);
    Indication.setSign(DynamicIndication::MSBMinutesTube, GPIOA, GPIO_PIN_4);
    Indication.setSign(DynamicIndication::LSBMinutesTube, GPIOA, GPIO_PIN_3);
    Indication.setNumber(1, 2, 3, 4);

    initWifi(wifi);

    Clock.setTimeZone(3);
    Clock.init(&wifi);

    for (;;) {
        const SString<64> inBuffer = usbHost.popData();
        if (!inBuffer.empty()) {
            processUsbCmd(inBuffer);
        }

        Clock.process();
        wifi.process();
    }
}