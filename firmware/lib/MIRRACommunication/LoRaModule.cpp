#include "LoRaModule.h"

using namespace mirra::comm;

LoRaModule::LoRaModule(uint8_t spreadingFactor)
    : module{pins::cs, pins::dio0, pins::rst}, SX1272(&module)
{
    this->module.setRfSwitchPins(pins::rx, pins::tx);
    int state =
        this->begin(frequency, spreadingFactor, codingRate, syncWord, power, preambleLength, gain);
    if (state == RADIOLIB_ERR_NONE)
    {
        Log::debug("LoRa init successful.");
    }
    else
    {
        Log::error("LoRa module init failed, code: ", state);
    }
};

void LoRaModule::setCompletionWake()
{
    canWait = true;
    esp_sleep_enable_ext0_wakeup((gpio_num_t)pins::dio0, 1);
}

void LoRaModule::setTimerWake(uint32_t ms)
{
    canWait = true;
    timerStart = esp_timer_get_time();
    esp_sleep_enable_timer_wakeup(ms * 1000);
}

void LoRaModule::clearWake()
{
    canWait = false;
    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
}

std::pair<bool, uint32_t> LoRaModule::wait()
{
    if (!canWait)
        return std::make_pair(false, 0);
    esp_light_sleep_start();
    standby();
    uint32_t timeDifferenceMs = (esp_timer_get_time() - timerStart) / 1000;
    return std::make_pair(esp_sleep_get_wakeup_cause() != ESP_SLEEP_WAKEUP_EXT0, timeDifferenceMs);
}

bool LoRaModule::sendPacket(const uint8_t* buffer, size_t size, uint32_t timeoutMs)
{
    clearWake();
    setCompletionWake();
    setTimerWake(timeoutMs);
    Log::debug("Starting send ...");
    int state = this->startTransmit(const_cast<uint8_t*>(buffer), size);
    if (state != RADIOLIB_ERR_NONE)
    {
        clearWake();
        Log::error("Send failed, code: ", state);
        return false;
    }
    return true;
}

bool LoRaModule::receivePacket(uint32_t timeoutMs)
{
    clearWake();
    setCompletionWake();
    setTimerWake(timeoutMs);
    Log::debug("Starting receive ...");
    int state = this->startReceive();
    if (state != RADIOLIB_ERR_NONE)
    {
        clearWake();
        Log::error("Receive failed, code: ", state);
        return false;
    }
    return true;
}
bool LoRaModule::readPacket(uint8_t* buffer)
{
    int state = readData(buffer, 0);
    if (state != RADIOLIB_ERR_NONE)
    {
        Log::error("Read failed, code: ", state);
        return false;
    }
    return true;
}