#include "MIRRAModule.h"

#include "logging.h"
#include <Arduino.h>
#include <Wire.h>
#include <ctime>

using namespace mirra;

void MIRRAModule::prepare()
{
    Serial.begin(115200);
    Serial.println("Serial initialised.");
    Serial.flush();
    pinMode(pins::peripheralPower, OUTPUT);
    digitalWrite(pins::peripheralPower, HIGH);
    gpio_hold_dis(static_cast<gpio_num_t>(pins::peripheralPower));
    Serial.println("Powering on peripherals...");
    // wait for power propagation
    esp_sleep_enable_timer_wakeup(10 * 1000);
    esp_light_sleep_start();
    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_TIMER);

    Wire.begin(pins::sda, pins::scl); // i2c
    pinMode(pins::boot, INPUT);
    Serial.println("I2C wire initialised.");
    fs::NVS::init();
    Serial.println("NVS initialsed.");
    Log::getInstance().serial = &Serial;
    Serial.println("Logger initialised.");
}

void MIRRAModule::end()
{
    Log::close();
    Wire.end();
    Serial.flush();
    Serial.end();
    digitalWrite(pins::peripheralPower, LOW);
    gpio_hold_en(static_cast<gpio_num_t>(pins::peripheralPower));
    gpio_deep_sleep_hold_en();
}
MIRRAModule::MIRRAModule() : rtc{pins::rtcInt, pins::rtcAddress}, commandEntry{pins::boot, true}
{
    esp_efuse_mac_get_default(mac.getAddress());
    Log::info("Reset reason: ", esp_rom_get_reset_reason(0));
}

MIRRAModule::SensorFile::SensorFile() : FIFOFile("data"), reader{nvs.getValue<size_t>("reader", 0)}
{}

size_t MIRRAModule::SensorFile::cutTail(size_t cutSize)
{
    size_t removed{0};
    while (removed < cutSize)
    {
        removed += DataEntry::getSize(read<DataEntry::Flags>(removed + DataEntry::flagsPosition));
    }
    reader = reader < removed ? 0 : reader - removed;
    return FIFOFile::cutTail(removed);
}

MIRRAModule::SensorFile::Iterator& MIRRAModule::SensorFile::Iterator::operator++()
{
    address += DataEntry::getSize(file->read<DataEntry::Flags>(address + DataEntry::flagsPosition));
    return *this;
}

std::optional<size_t> MIRRAModule::SensorFile::getUnuploadedAddress(size_t index)
{
    size_t address = reader;
    size_t count{0};
    while (true)
    {
        if (address == getSize())
            return std::nullopt;
        DataEntry::Flags flags = read<DataEntry::Flags>(address + DataEntry::flagsPosition);
        if (!flags.uploaded)
        {
            if (count == 0)
                reader = address;
            if (count == index)
                return address;
            count++;
        }
        address += DataEntry::getSize(flags);
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

bool MIRRAModule::SensorFile::isLast(size_t index)
{
    return !getUnuploadedAddress(index + 1);
}

void MIRRAModule::SensorFile::push(const comm::Message<comm::DATA>& message)
{
    FIFOFile::push(DataEntry{message.getHeader().addr, message.body.time,
                             DataEntry::Flags{message.body.nValues, false}, message.body.values});
}

void MIRRAModule::SensorFile::setUploaded()
{
    DataEntry::Flags flags = read<DataEntry::Flags>(reader + DataEntry::flagsPosition);
    flags.uploaded = true;
    write(reader + DataEntry::flagsPosition, flags);
}

void MIRRAModule::SensorFile::flush()
{
    reader.commit();
    FIFOFile::flush();
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
    esp_sleep_enable_ext1_wakeup((gpio_num_t)_BV(pins::boot),
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
        Log::getInstance().file.level = Log::Level::DEBUG;
    else if (strcmp("INFO", arg) == 0)
        Log::getInstance().file.level = Log::Level::INFO;
    else if (strcmp("ERROR", arg) == 0)
        Log::getInstance().file.level = Log::Level::ERROR;
    else
    {
        Serial.printf("Argument '%s' is not a valid log level.\n", arg);
        return COMMAND_ERROR;
    }
    return COMMAND_SUCCESS;
}

CommandCode MIRRAModule::Commands::printLogs()
{
    static constexpr size_t bufferSize{256};
    char buffer[bufferSize];
    size_t cursor{0};
    const Log::File& file = Log::getInstance().file;
    Serial.printf("Logs: %u out of %u KB.\n", file.getSize() / 1024, file.getMaxSize() / 1024);
    while (cursor < file.getSize())
    {
        file.read(cursor, buffer, bufferSize);
        Serial.write(buffer, std::min(bufferSize, file.getSize() - cursor));
        cursor += bufferSize;
    }
    Serial.print('\n');
    return COMMAND_SUCCESS;
}

CommandCode MIRRAModule::Commands::printData()
{
    SensorFile file{};
    Serial.printf("Data: %u out of %u KB.\n", file.getSize() / 1024, file.getMaxSize() / 1024);
    for (SensorFile::DataEntry entry : file)
    {
        Serial.printf("%s ", entry.source.toString());

        time_t time = static_cast<time_t>(entry.time);
        static constexpr size_t timeLength{sizeof("0000-00-00 00:00:00")};
        char timeBuffer[timeLength];
        std::strftime(timeBuffer, timeLength, "%F %T", gmtime(&time));
        Serial.print(timeBuffer);

        if (entry.flags.uploaded)
            Serial.print(" UP");

        Serial.print("\n");

        for (size_t i = 0; i < entry.flags.nValues; i++)
        {
            Serial.printf("%u %f\n", entry.values[i].typeTag, entry.values[i].value);
        }

        Serial.print("\n");
    }
    return COMMAND_SUCCESS;
}

CommandCode MIRRAModule::Commands::printDataRaw()
{
    SensorFile file{};
    for (SensorFile::DataEntry entry : file)
    {
        for (size_t i = 0; i < entry.getSize(); i++)
        {
            Serial.printf("%02X", reinterpret_cast<uint8_t*>(&entry)[i]);
        }
        Serial.print('\n');
    }
    return COMMAND_SUCCESS;
}

CommandCode MIRRAModule::Commands::format()
{
    Serial.println("Erasing NVS...");
    nvs_flash_erase();
    Serial.println("Restarting...");
    ESP.restart();
    return COMMAND_SUCCESS;
}

CommandCode MIRRAModule::Commands::spam(size_t count)
{
    for (size_t i = 0; i < count; i++)
        Log::getInstance().file.push("abcdefghijklmnopqrstuvwxyz\n");
    Serial.println("Spamming done.");
    return COMMAND_SUCCESS;
}
