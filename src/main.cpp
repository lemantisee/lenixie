#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>

#include "BCDDecoder.h"
#include "DynamicIndication.h"
#include "RTClock.h"
#include "ESP9266.h"
#include "SMProtocol/SMProtocol.h"
#include "NTPHandle.h"

namespace
{
    constexpr uint32_t Fcpu = 72000000;

    volatile uint16_t rtcTubePeriod = 0;
    bool updateClock = false;
    volatile bool testBlinkStart = false;
    volatile uint16_t testTimer = 0;
    volatile uint32_t ntpPeriod = 43200000; //12 hours in ms

    BCDDecoder dec;
    DynamicIndication Indication;
    RTClock Clock;
    ESP9266 wifi;
    SMProtocol protocol;
    NTPHandle ntpRequest;

    enum Command
    {
        DeviceID     = 100,
        SetWifi      = 101,
        SetNtpServer = 102,
        SetTime      = 103,
        ResetFabric  = 104,
        TestBlink    = 105,
        EnableNTP	 = 106,
        SetTimeZone	 = 107
    };

    void updateIndication() 
    {
        const RTClock::Time &time = Clock.getTime();
        Indication.setNumber(DynamicIndication::MSBHourTube, time.hours / 10);
        Indication.setNumber(DynamicIndication::LSBHourTube, time.hours % 10);
        Indication.setNumber(DynamicIndication::MSBMinutesTube, time.minutes / 10);
        Indication.setNumber(DynamicIndication::LSBMinutesTube, time.minutes % 10);
    }
} // namespace

extern "C" {
    void sys_tick_handler() 
    {
        wifi.processTimer();
        Indication.process();
        ++rtcTubePeriod;
        ntpPeriod--;
        if (rtcTubePeriod == 1000) {
            rtcTubePeriod = 0;
            updateIndication();
            updateClock ^= 1;
        }
    }

    void usart1_isr() 
    {
        wifi.processUART();
    }
}

int main(void)
{
    rcc_clock_setup_in_hse_8mhz_out_72mhz();
    rcc_periph_clock_enable(RCC_GPIOA);
    rcc_periph_clock_enable(RCC_GPIOB);
    rcc_periph_clock_enable(RCC_USART1);
    rcc_periph_clock_enable(RCC_USART3);
    rcc_periph_clock_enable(RCC_AFIO);
    rcc_periph_clock_enable(RCC_PWR);
    rcc_periph_clock_enable(RCC_BKP);

    systick_counter_enable();
    systick_interrupt_enable();
    systick_set_reload(F_CPU/1000-1);

    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_OPENDRAIN, GPIO8);

    dec.init(GPIOB, GPIO6, GPIO8, GPIO9, GPIO7);

    Indication.setNumbersPin(&dec);
    Indication.setSign(DynamicIndication::MSBHourTube, GPIOA, GPIO6);
    Indication.setSign(DynamicIndication::LSBHourTube, GPIOA, GPIO5);
    Indication.setSign(DynamicIndication::MSBMinutesTube, GPIOA, GPIO4);
    Indication.setSign(DynamicIndication::LSBMinutesTube, GPIOA, GPIO3);
    Indication.setNumber(DynamicIndication::MSBHourTube, 1);
    Indication.setNumber(DynamicIndication::LSBHourTube, 2);
    Indication.setNumber(DynamicIndication::MSBMinutesTube, 3);
    Indication.setNumber(DynamicIndication::LSBMinutesTube, 4);
    Indication.startIndication(true);

    Clock.init();
    Clock.setTimeZone(3);

    wifi.init(USART1, GPIOA, GPIO10, GPIO9, 115200);
    wifi.switchToAP();

    protocol.init(&wifi);
    ntpRequest.init(&wifi);

    
    if (!wifi.isConnected()) {
        // Logger::instance().log("Cannot connect ot wifi\n");
        wifi.switchToAP();
    } else {
        // Logger::instance().log("Connected ot wifi\n");
        if (ntpRequest.process("0.pool.ntp.org")) {
            Clock.setTime(ntpRequest.getHours(), ntpRequest.getMinutes(), ntpRequest.getSeconds());
        }
    }

    wifi.getIP();
    wifi.clearBuffer();

    for (;;) {
        
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
        
        if (protocol.isCommandReady()) {
            // Logger::instance().log("Recieve UDP\n");

            switch (Command(protocol.getCommand())) {
            case DeviceID:
                // Logger::instance().log("Recieve command DeviceID\n");
                //wifi.SendString("100:NxC_2", "192.168.4.255", 1111);
                wifi.SendString("100:NxC_2");
                wifi.SendString("100:NxC_2");
                break;
            case SetWifi: {
                // Logger::instance().log("Setting new wifi network\n");
                wifi.SendString("101:Ok");
                char *wStr = (char *)protocol.getStringParametr();
                // Logger::instance().log("New wifi:");
                // Logger::instance().log(wStr);
                const char *ssid = strsep(&wStr, ",");
                const char *pass = strsep(&wStr, ",");
                wifi.closeCurrentConnection();
                if (!wifi.connectNetwork(ssid, pass)) {
                    wifi.switchToAP();
                }
                else {
                    wifi.getIP();
                    wifi.connectToServerUDP(wifi.broadcastIP, 1111);
                    wifi.SendString("100:NxC_2");
                    if (ntpRequest.process("0.pool.ntp.org")) {
                        Clock.setTime(ntpRequest.getHours(), ntpRequest.getMinutes(), ntpRequest.getSeconds());
                    }
                }
                wifi.clearBuffer();
                } break;
            case SetTime: {
                // Logger::instance().log("Setting new time\n");
                wifi.SendString("103:Ok");
                char *wStr = (char *)protocol.getStringParametr();
                const char *str1 = strsep(&wStr, ",");
                const char *str2 = strsep(&wStr, ",");
                wifi.clearBuffer();
                Clock.setTime(atoi(str1), atoi(str2), 0);
                // testH = Clock.getTime()->RTC_Hours;
                // testM = Clock.getTime()->RTC_Minutes;
                } break;
            case SetNtpServer:{
                // Logger::instance().log("Setting new NTP server\n");
                wifi.SendString("102:Ok");
                const char *wStr = (char *)protocol.getStringParametr();
                if (ntpRequest.process(wStr)) {
                    Clock.setTime(ntpRequest.getHours(), ntpRequest.getMinutes(), ntpRequest.getSeconds());
                }
                wifi.clearBuffer();
                } break;
            case TestBlink: {
                // Logger::instance().log("Activate test blink\n");
                wifi.SendString("105:Ok");
                const uint8_t parametr = protocol.getParametr();
                wifi.clearBuffer();
                if (parametr) {
                    testBlinkStart = true;
                } else {
                    testBlinkStart = false;
                    Indication.startIndication(true);
                }
                } break;
            case SetTimeZone: {         
                wifi.SendString("107:Ok");
                // Logger::instance().log("Setting new time zone\n");{
                const char *wStr = (char *)protocol.getStringParametr();
                wifi.clearBuffer();
                const uint8_t timeZone = atoi(wStr);
                Clock.setTimeZone(timeZone);
                } break;
            default:
                wifi.clearBuffer();
                break;
            }
        }
    }
}