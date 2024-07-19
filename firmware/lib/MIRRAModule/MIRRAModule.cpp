#include "MIRRAModule.h"

#include "logging.h"
#include <Arduino.h>
#include <Wire.h>

using namespace mirra;

void MIRRAModule::prepare(const MIRRAPins& pins)
{
    Serial.begin(115200);
    Serial.println("Serial initialised.");
    gpio_hold_dis(static_cast<gpio_num_t>(pins.peripheralPowerPin));
    pinMode(pins.peripheralPowerPin, OUTPUT);
    digitalWrite(pins.peripheralPowerPin, HIGH);
    Wire.begin(pins.sdaPin, pins.sclPin); // i2c
    pinMode(pins.bootPin, INPUT);
    Serial.println("I2C wire initialised.");
    fs::NVS::init();
    Serial.println("NVS initialsed.");
}

void MIRRAModule::end()
{
    Log::end();
    lora.sleep();
    Wire.end();
    Serial.flush();
    Serial.end();
    digitalWrite(pins.peripheralPowerPin, LOW);
    gpio_hold_en(static_cast<gpio_num_t>(pins.peripheralPowerPin));
    gpio_deep_sleep_hold_en();
}
MIRRAModule::MIRRAModule(const MIRRAPins& pins)
    : pins{pins}, rtc{pins.rtcIntPin, pins.rtcAddress},
      lora{pins.csPin, pins.rstPin, pins.dio0Pin, pins.rxPin, pins.txPin},
      commandEntry{pins.bootPin, true}
{
    Log::log.serial = &Serial;
    Log::init();
    Serial.println("Logger initialised.");
    Log::info("Reset reason: ", esp_rom_get_reset_reason(0));
}

MIRRAModule::SensorFile::SensorFile() : FIFOFile("data"), reader{nvs.getValue<size_t>("reader", 0)}
{
}

void MIRRAModule::SensorFile::flush()
{
    reader.commit();
    FIFOFile::flush();
}

size_t MIRRAModule::SensorFile::cutTail(size_t cutSize)
{
    size_t removed{0};
    while (removed < cutSize)
    {
        removed +=
            DataEntry::getSize(read<Message<SENSOR_DATA>::Flags>(removed + sizeof(MACAddress)));
    }
    reader = reader < removed ? 0 : reader - removed;
    return FIFOFile::cutTail(removed);
}

std::optional<size_t> MIRRAModule::SensorFile::getUnuploadedAddress(size_t index)
{
    size_t address = reader;
    size_t count{0};
    while (true)
    {
        if (address == getSize())
            return std::nullopt;
        DataEntry::Flags flags = read<DataEntry::Flags>(address + sizeof(MACAddress));
        address += DataEntry::getSize(flags);
        if (!flags.uploaded)
        {
            if (count == 0)
                reader = address;
            if (count == index)
                return address;
            count++;
        }
    }
}

std::optional<MIRRAModule::SensorFile::DataEntry>
MIRRAModule::SensorFile::getUnuploaded(size_t index)
{
    auto address = getUnuploadedAddress(index);
    if (!address)
        return std::nullopt;
    return read<DataEntry>(*address);
}
bool MIRRAModule::SensorFile::isLast(size_t index) { return !getUnuploadedAddress(index + 1); }

void MIRRAModule::SensorFile::setUploaded()
{
    DataEntry::Flags flags = read<DataEntry::Flags>(reader + sizeof(MACAddress));
    flags.uploaded = true;
    write(reader + sizeof(MACAddress), flags);
}

void MIRRAModule::SensorFile::push(const Message<SENSOR_DATA>& message)
{
    FIFOFile::push(DataEntry{message.getSource(), message.flags, message.time, message.values});
}

void MIRRAModule::deepSleep(uint32_t sleepTime)
{
    if (sleepTime <= 0)
    {
        Log::error("Sleep time was zero or negative! Sleeping one second to avert crisis.");
        sleepTime = 1;
    }

    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
    // The external RTC only has a alarm resolution of 1s, to be more accurate for times lower than
    // 30s the internal oscillator will be used to wake from deep sleep
    if (sleepTime <= 30)
    {
        Log::debug("Using internal timer for deep sleep.");
        esp_sleep_enable_timer_wakeup((uint64_t)sleepTime * 1000 * 1000);
    }
    else
    {
        Log::debug("Using RTC for deep sleep.");
        rtc.writeAlarm(rtc.readTimeEpoch() + sleepTime);
        rtc.enableAlarm();
        esp_sleep_enable_ext0_wakeup((gpio_num_t)rtc.getIntPin(), 0);
    }
    esp_sleep_enable_ext1_wakeup((gpio_num_t)_BV(this->pins.bootPin),
                                 ESP_EXT1_WAKEUP_ALL_LOW); // wake when BOOT button is pressed
    Log::info("Good night.");
    this->end();
    esp_deep_sleep_start();
}

void MIRRAModule::deepSleepUntil(uint32_t untilTime)
{
    uint32_t cTime{rtc.getSysTime()};
    if (untilTime <= cTime)
    {
        deepSleep(0);
    }
    else
    {
        deepSleep(untilTime - cTime);
    }
}

void MIRRAModule::lightSleep(float sleepTime)
{
    if (sleepTime <= 0)
    {
        Log::error("Sleep time was zero or negative! Skipping to avert crisis.");
        return;
    }
    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
    esp_sleep_enable_timer_wakeup((uint64_t)sleepTime * 1000 * 1000);
    esp_light_sleep_start();
}

void MIRRAModule::lightSleepUntil(uint32_t untilTime)
{
    uint32_t cTime{rtc.getSysTime()};
    if (untilTime <= cTime)
    {
        return;
    }
    else
    {
        lightSleep(untilTime - cTime);
    }
}

CommandCode MIRRAModule::Commands::setLogLevel(const char* arg)
{
    if (strcmp("DEBUG", arg) == 0)
        Log::log.file.level = Log::Level::DEBUG;
    else if (strcmp("INFO", arg) == 0)
        Log::log.file.level = Log::Level::INFO;
    else if (strcmp("ERROR", arg) == 0)
        Log::log.file.level = Log::Level::ERROR;
    else
    {
        Serial.printf("Argument '%s' is not a valid log level.\n", arg);
        return COMMAND_ERROR;
    }
    return COMMAND_SUCCESS;
}