#ifndef __CONFIG_H__
#define __CONFIG_H__

// Pin assignments

#define BOOT_PIN 0

#define PERIPHERAL_POWER_PIN 16

#define SDA_PIN 21
#define SCL_PIN 22

// LoRa module pins
#define CS_PIN 27   // Chip select pin
#define RST_PIN 5   // Reset pin
#define DIO0_PIN 13 // DIO0 pin: LoRa interrupt pin
#define TX_PIN 12
#define RX_PIN 26

// PCF2129 timer
#define RTC_INT_PIN 35   // Interrupt pin
#define RTC_ADDRESS 0x51 // i2c address

// Battery pins
#define BATT_PIN 34
#define BATT_EN_PIN 33

#endif
