#ifndef __RADIO_H__
#define __RADIO_H__

#include <Arduino.h>
#include <CommunicationCommon.h>
#include <PCF2129_RTC.h>
#include <Pins.h>
#include <RadioLib.h>
#include <logging.h>

#include <optional>

namespace mirra::comm
{
/// @brief Responsible for low-level LoRa communication and control over the SX1272 module
class LoRaModule : SX1272
{
private:
    static constexpr float frequency{886.0};
    static constexpr float bandwith{125.0};
    static constexpr uint8_t codingRate{6};
    static constexpr uint8_t syncWord{0x12};
    static constexpr uint8_t power{10};
    static constexpr uint16_t preambleLength{8};
    static constexpr uint8_t gain{0}; // 0 is automatic

    Module module;

    bool canWait{false};
    int64_t timerStart;

    void setCompletionWake();
    void setTimerWake(uint32_t ms);
    void clearWake();

public:
    LoRaModule(uint8_t spreadingFactor);
    ~LoRaModule() { sleep(); }

    bool sendPacket(const uint8_t* buffer, size_t size, uint32_t timeoutMs);

    bool receivePacket(uint32_t timeoutMs);

    bool readPacket(uint8_t* buffer);

    /// @brief Wait until receive/send has been successfully completed or until timeout.
    /// @return Pair with timed out state (true if timed out)
    std::pair<bool, uint32_t> wait();

    friend class Protocol;
};
}

#endif
