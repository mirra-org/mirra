#include "config.h"
#include "gateway.h"

void setup(void)
{
    esp_sleep_enable_timer_wakeup(100 * 1000);
    while (true)
    {
        Serial.printf("%i | %i", esp_timer_get_time() ,esp_timer_get_next_alarm_for_wake_up())
        delay(10)
    }


    constexpr MIRRAModule::MIRRAPins pins = {.bootPin = BOOT_PIN,
                                             .peripheralPowerPin = PERIPHERAL_POWER_PIN,
                                             .sdaPin = SDA_PIN,
                                             .sclPin = SCL_PIN,
                                             .csPin = CS_PIN,
                                             .rstPin = RST_PIN,
                                             .dio0Pin = DIO0_PIN,
                                             .txPin = TX_PIN,
                                             .rxPin = RX_PIN,
                                             .rtcIntPin = RTC_INT_PIN,
                                             .rtcAddress = RTC_ADDRESS};
    MIRRAModule::prepare(pins);
    Gateway gateway{pins};
    gateway.wake();
}

void loop(void) {}
