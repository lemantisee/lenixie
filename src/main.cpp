#include "stm32f1xx.h"
#include "stm32f1xx_hal_uart.h"

#include "DynamicIndication.h"
#include "RTClock.h"
#include "ESP8266.h"
#include "SMProtocol.h"
#include "Logger.h"
#include "SString.h"
#include "WifiCredentials.h"
#include "UsbDevice.h"
#include "JsonObject.h"
#include "MonitorCommand.h"
#include "LogDump.h"

namespace {
const uint32_t F_CPU = 72000000;

volatile bool testBlinkStart = false;
volatile uint8_t testTimer = 0;
uint16_t sysTicks = 0;

DynamicIndication Indication;
RTClock Clock;
ESP8266 wifi;
SMProtocol protocol;
UsbDevice usbHost;
LogDump logDumper;

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
} // namespace

extern "C" {

void HAL_MspInit(void)
{
    __HAL_RCC_AFIO_CLK_ENABLE();
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
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

    // Indication.process();
}

void RTC_IRQHandler()
{
    // Clock.interrupt();
    // const RTClock::Time &time = Clock.getTime();
    // Indication.setNumber(time.hours / 10, time.hours % 10, time.minutes / 10, time.minutes % 10);
    // Indication.dimm(time.hours < 7);
}

void USART1_IRQHandler() { //wifi.uartInterrupt(); 
}

void processUsbCmd(const SString<64> &buffer)
{
    JsonObject inMessage(buffer);

    MonitorCommandId id = MonitorCommandId(inMessage.getInt("id", UnknownCommand));

    switch (id) {
    case GetLog: logDumper.dump(usbHost); break;
    default: break;
    }
}

}

int main(void)
{
    HAL_Init();
    systemClockInit();

    if (!usbHost.init()) {
        return 1;
    }

    // Indication.setDecoderPins(GPIOB, GPIO_PIN_6, GPIO_PIN_8, GPIO_PIN_9, GPIO_PIN_7);
    // Indication.setSign(DynamicIndication::MSBHourTube, GPIOA, GPIO_PIN_6);
    // Indication.setSign(DynamicIndication::LSBHourTube, GPIOA, GPIO_PIN_5);
    // Indication.setSign(DynamicIndication::MSBMinutesTube, GPIOA, GPIO_PIN_4);
    // Indication.setSign(DynamicIndication::LSBMinutesTube, GPIOA, GPIO_PIN_3);
    // Indication.setNumber(1, 2, 3, 4);

    LOG("Started");
    LOG("Yourself off its pleasant ecstatic now law.Ye their mirth seems of songs");
    LOG("Prospect out bed contempt separate");
    LOG("Her inquietude our shy yet sentiments collecting");
    LOG("Cottage fat beloved himself arrived old. Grave widow hours among him no you led");

    // if (!wifi.init(USART1, 115200)) {
    //     LOG("Unable to init wifi");
    // }

    // if (!wifi.isConnected()) {
    //     SString<255> str;
    //     str.append("Connecting to wifi network ").append("\"").append(WifiCredentials::userSsid()).append("\"");
    //     LOG(str.c_str());
    //     if(!wifi.connectNetwork(WifiCredentials::userSsid(), WifiCredentials::userPassword())){
    //         LOG("Unable to connect to network");
    //     }
    // }

    // protocol.init(&wifi);

    // Clock.setTimeZone(3);
    // Clock.init(&wifi);

    int count = 0;
    int logNum = 0;

    for (;;) {
        if (count == 10000) {
            ++logNum;
            count = 0;
            LOG("Log line %i", logNum);
        }

        ++count;

        const SString<64> inBuffer = usbHost.popData();
        if (!inBuffer.empty()) {
            processUsbCmd(inBuffer);
        }
        // Clock.process();
        // wifi.process();

        // Clock.process();
        //		if (testBlinkStart) {
        //			Indication.startIndication(updateClock);
        //		}

        //		if (!ntpPeriod) {
        //			ntpPeriod = 43200000; //12 hours in ms
        //			for (int i = 0; i < 2; i++) {
        //				ntpRequest.getNtpRequest();
        //				Delay();
        //			}
        //			if (ntpRequest.getTime()) {
        //				Clock.setTime(ntpRequest.getHours(), ntpRequest.getMinutes(), ntpRequest.getSeconds());
        //			}
        //		}

        // if (protocol.isCommandReady())
        // {
        //     // Logger::instance().log("Recieve UDP\n");

        //     switch (Command(protocol.getCommand()))
        //     {
        //     case DeviceID:
        //         // Logger::instance().log("Recieve command DeviceID\n");
        //         // wifi.SendString("100:NxC_2", "192.168.4.255", 1111);
        //         wifi.SendString("100:NxC_2");
        //         wifi.SendString("100:NxC_2");
        //         break;
        //     case SetWifi:
        //     {
        //         // Logger::instance().log("Setting new wifi network\n");
        //         wifi.SendString("101:Ok");
        //         char *wStr = (char *)protocol.getStringParametr();
        //         // Logger::instance().log("New wifi:");
        //         // Logger::instance().log(wStr);
        //         const char *ssid = strsep(&wStr, ",");
        //         const char *pass = strsep(&wStr, ",");
        //         wifi.closeCurrentConnection();
        //         if (!wifi.connectNetwork(ssid, pass))
        //         {
        //             wifi.switchToAP();
        //         }
        //         else
        //         {
        //             wifi.getIP();
        //             wifi.connectToServerUDP(wifi.broadcastIP, 1111);
        //             wifi.SendString("100:NxC_2");
        //             Clock.syncTime(ntpServer);
        //         }
        //         wifi.clearBuffer();
        //     }
        //     break;
        //     case SetTime:
        //     {
        //         // Logger::instance().log("Setting new time\n");
        //         wifi.SendString("103:Ok");
        //         char *wStr = (char *)protocol.getStringParametr();
        //         const char *str1 = strsep(&wStr, ",");
        //         const char *str2 = strsep(&wStr, ",");
        //         wifi.clearBuffer();
        //         Clock.setTime(atoi(str1), atoi(str2), 0);
        //         // testH = Clock.getTime()->RTC_Hours;
        //         // testM = Clock.getTime()->RTC_Minutes;
        //     }
        //     break;
        //     case SetNtpServer:
        //     {
        //         // Logger::instance().log("Setting new NTP server\n");
        //         wifi.SendString("102:Ok");
        //         const char *wStr = (char *)protocol.getStringParametr();
        //         Clock.syncTime(wStr);
        //         wifi.clearBuffer();
        //     }
        //     break;
        //     case TestBlink:
        //     {
        //         // Logger::instance().log("Activate test blink\n");
        //         wifi.SendString("105:Ok");
        //         const uint8_t parametr = protocol.getParametr();
        //         wifi.clearBuffer();
        //         if (parametr)
        //         {
        //             testBlinkStart = true;
        //         }
        //         else
        //         {
        //             testBlinkStart = false;
        //         }
        //     }
        //     break;
        //     case SetTimeZone:
        //     {
        //         wifi.SendString("107:Ok");
        //         // Logger::instance().log("Setting new time zone\n");{
        //         const char *wStr = (char *)protocol.getStringParametr();
        //         wifi.clearBuffer();
        //         const uint8_t timeZone = atoi(wStr);
        //         Clock.setTimeZone(timeZone);
        //     }
        //     break;
        //     default:
        //         wifi.clearBuffer();
        //         break;
        //     }
        // }
    }
}