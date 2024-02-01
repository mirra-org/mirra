#ifndef __RADIO_H__
#define __RADIO_H__

#include <Arduino.h>
#include <CommunicationCommon.h>
#include <PCF2129_RTC.h>
#include <RadioLib.h>
#include <logging.h>

#include <optional>

/******************************
 *     LoRa configuration
 *****************************/
#define LORA_FREQUENCY 866.0
#define LORA_BANDWIDTH 125.0
#define LORA_SPREADING_FACTOR 7
#define LORA_CODING_RATE 6
#define LORA_SYNC_WORD 0x12
#define LORA_POWER 10
#define LORA_PREAMBLE_LENGHT 8
#define LORA_AMPLIFIER_GAIN 0 // 0 is automatic

#define SEND_DELAY 500 // ms, time to wait before sending a message

/// @brief Responsible for LoRa communication and control over the SX1272 module
class LoRaModule : SX1272
{
private:
    Module module;

    /// @brief Pin number for SX1272's DIO0 interrupt pin
    uint8_t DIO0Pin;

    bool canWait{false};

public:
    /// @brief Constructs a LoRaModule with the given pin parameters
    /// @param csPin Chip select pin
    /// @param rstPin Reset pin
    /// @param DIOPin DIO0 interrupt pin
    /// @param rxPin Receive pin
    /// @param txPin Transmit pin
    LoRaModule(const uint8_t csPin, const uint8_t rstPin, const uint8_t DIOPin, const uint8_t rxPin, const uint8_t txPin);

    void setCompletionWake();
    void setTimerWake(uint32_t ms);
    void clearWake();
    bool wait();

    bool sendPacket(const uint8_t* buffer, size_t size);
    /// @brief Resends the last sent message stored in the sendBuffer. If there is none, does nothing.

    /// @brief Receives a specific type of message from a specific source. When timing out, sends a REPEAT message according to the repeatAttempts parameter.
    /// @tparam T Desired type of the message
    /// @param timeoutMs The amount of time in ms to listen for a message per repeat attempt.
    /// @param repeatAttempts The amount of times to send a repeat message before definitively timing out.
    /// @param src Expected source of the message. Set to the broadcast MAC address to catch all sources.
    /// @param listenMs The amount of extra time in ms to listen during the very fist repeat attempt. Useful when attempting to catch the first message in a
    /// 'conversation'.
    /// @param promiscuous Whether to catch messages with a destination MAC not set to this specific module. (Note: messages with the broadcast MAC as
    /// destination will be caught regardless)

    bool receivePacket(uint32_t timeoutMs);
    bool readPacket(uint8_t* buffer, size_t size);
};

#include <LoRaModule.tpp>

#endif
