#include "LoRaModule.h"

LoRaModule::LoRaModule(const uint8_t csPin, const uint8_t rstPin, const uint8_t DIO0Pin, const uint8_t rxPin, const uint8_t txPin)
    : module{csPin, DIO0Pin, rstPin}, DIO0Pin{DIO0Pin}, SX1272(&module)
{
    this->module.setRfSwitchPins(rxPin, txPin);
    esp_efuse_mac_get_default(this->mac.getAddress());
    int state = this->begin(LORA_FREQUENCY, LORA_BANDWIDTH, LORA_SPREADING_FACTOR, LORA_CODING_RATE, LORA_SYNC_WORD, LORA_POWER, LORA_PREAMBLE_LENGHT,
                            LORA_AMPLIFIER_GAIN);
    if (state == RADIOLIB_ERR_NONE)
    {
        Log::debug("LoRa init successful for ", this->getMACAddress().toString());
    }
    else
    {
        Log::error("LoRa module init failed, code: ", state);
    }
};

void LoRaModule::setCompletionWake()
{
    canWait = true;
    esp_sleep_enable_ext0_wakeup((gpio_num_t)DIO0Pin, 1);
}

void LoRaModule::setTimerWake(uint32_t ms)
{
    canWait = true;
    esp_sleep_enable_timer_wakeup(ms * 1000);
}

void LoRaModule::clearWake()
{
    canWait = false;
    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
}

bool LoRaModule::wait()
{
    if (!canWait)
        return true;
    esp_light_sleep_start();
    clearWake();
    return esp_sleep_get_wakeup_cause() != ESP_SLEEP_WAKEUP_TIMER;
}

bool LoRaModule::sendPacket(const uint8_t* buffer, size_t size)
{
    clearWake();
    setCompletionWake();
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
    // When the LoRa module get's a message it will generate an interrupt on DIO0.
    setCompletionWake();
    // We use the timer wakeup as timeout for receiving a LoRa reply.
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