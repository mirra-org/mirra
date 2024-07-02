#ifndef __RADIO_H__
#define __RADIO_H__

#include <Arduino.h>
#include <MIRRACommunicationCommon.h>
#include <PCF2129_RTC.h>
#include <RadioLib.h>
#include <logging.h>

#include <optional>

/// @brief Responsible for low-level LoRa communication and control over the SX1272 module
class LoRaModule : SX1272
{
private:
    static constexpr float frequency{886.0};
    static constexpr float bandwith{125.0};
    static constexpr uint8_t spreadingFactor{7};
    static constexpr uint8_t codingRate{6};
    static constexpr uint8_t syncWord{0x12};
    static constexpr uint8_t power{10};
    static constexpr uint16_t preambleLength{8};
    static constexpr uint8_t gain{0}; // 0 is automatic

    static constexpr uint32_t defaultSendTimeoutMs{500};
    static constexpr uint32_t defaultRecvTimeoutMs{1500};

    Module module;

    /// @brief Pin number for SX1272's DIO0 interrupt pin
    uint8_t DIO0Pin;

    bool canWait{false};
    uint64_t timeoutUs;

    void setCompletionWake();
    void setTimerWake(uint32_t ms);
    void clearWake();

public:
    /// @brief Constructs a LoRaModule with the given pin parameters
    /// @param csPin Chip select pin
    /// @param rstPin Reset pin
    /// @param DIOPin DIO0 interrupt pin
    /// @param rxPin Receive pin
    /// @param txPin Transmit pin
    LoRaModule(uint8_t csPin, uint8_t rstPin, uint8_t DIOPin, uint8_t rxPin, uint8_t txPin);

    bool sendPacket(const uint8_t* buffer, size_t size, uint32_t timeoutMs = defaultSendTimeoutMs);

    bool receivePacket(uint32_t timeoutMs = defaultRecvTimeoutMs);

    bool readPacket(uint8_t* buffer);

    /// @brief Wait until receive/send has been successfully completed or until timeout.
    /// @return True when message succesfully sent/received, false if timed out.
    bool wait();
};

#endif
