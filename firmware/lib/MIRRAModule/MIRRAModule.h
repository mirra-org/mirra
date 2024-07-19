#ifndef __MIRRAMODULE_H__
#define __MIRRAMODULE_H__

#include "Commands.h"
#include "FS.h"
#include "LoRaModule.h"
#include "PCF2129_RTC.h"

namespace mirra
{

/// @brief A base class for MIRRA modules to inherit from, which implements common functionality.
class MIRRAModule
{
public:
    /// @brief Struct that groups pin configurations for the initialisation of a MIRRAModule.
    struct MIRRAPins
    {
        uint8_t bootPin;
        uint8_t peripheralPowerPin;
        uint8_t sdaPin;
        uint8_t sclPin;
        uint8_t csPin;
        uint8_t rstPin;
        uint8_t dio0Pin;
        uint8_t txPin;
        uint8_t rxPin;
        uint8_t rtcIntPin;
        uint8_t rtcAddress;
    };
    /// @brief Configures the ESP32's pins, starts basic communication primitives (UART, I2C) and
    /// mounts the filesystem. Must be called before initialisation of the MIRRAModule.
    /// @param pins The pin configuration for the MIRRAModule.
    static void prepare(const MIRRAPins& pins);

protected:
    /// @brief Initialises the MIRRAModule, RTC, LoRa and logging modules.
    /// @param pins The pin configuration for the MIRRAModule.
    MIRRAModule(const MIRRAPins& pins);

    MIRRAModule(const MIRRAModule&) = delete;
    MIRRAModule& operator=(const MIRRAModule&) = delete;
    ~MIRRAModule() = default;

    struct Commands : CommonCommands
    {
        /// @brief Change the log level.
        /// @param arg String describing the new log level. ("ERROR", "INFO" or "DEBUG")
        CommandCode setLogLevel(const char* arg);

        static constexpr auto getCommands()
        {
            return std::tuple_cat(CommonCommands::getCommands(),
                                  std::make_tuple(CommandAliasesPair(&Commands::setLogLevel,
                                                                     "setlog", "setloglevel")));
        }
    };

    class SensorFile final : fs::FIFOFile
    {
        fs::NVS::Value<size_t> reader;

        size_t cutTail(size_t cutSize);

    public:
        struct DataEntry
        {
            using Flags = Message<SENSOR_DATA>::Flags;
            using SensorValueArray = Message<SENSOR_DATA>::SensorValueArray;

            MACAddress source;
            Flags flags;
            uint32_t time;
            SensorValueArray values;

            static constexpr size_t getSize(Flags flags)
            {
                return sizeof(source) + sizeof(flags) + sizeof(time) +
                       flags.nValues * sizeof(SensorValue);
            }
            constexpr size_t getSize() { return getSize(flags); }
        } __attribute__((packed));

        SensorFile();
        using FIFOFile::getSize;
        void push(const Message<SENSOR_DATA>& message);
        void push(const DataEntry& entry) { push(entry); }
        /* void push(const MACAddress& source, uint32_t time, uint8_t nValues,
                  const std::array<SensorValue, Message<SENSOR_DATA>::maxNValues> values); */

        std::optional<size_t> getUnuploadedAddress(size_t index);
        std::optional<DataEntry> getUnuploaded(size_t index);
        bool isLast(size_t index);

        void setUploaded();
        using FIFOFile::read;

        void flush();
    };

    /// @brief Enters deep sleep for the specified time.
    /// @param sleepTime The time in seconds to sleep.
    void deepSleep(uint32_t sleepTime);
    /// @brief Enters deep sleep until the specified time.
    /// @param untilTime The time (UNIX epoch, seconds) the module should wake.
    void deepSleepUntil(uint32_t untilTime);
    /// @brief Enters light sleep for the specified time.
    /// @param sleepTime The time in seconds to sleep.
    void lightSleep(float sleepTime);
    /// @brief Enters light sleep until the specified time.
    /// @param untilTime The time (UNIX epoch, seconds) the module should wake.
    void lightSleepUntil(uint32_t untilTime);

    /// @brief Gracefully shuts down the dependencies. This function can be thought of as a
    /// counterpoint to MIRRAModule::prepare.
    /// @see MIRRAModule::prepare
    void end();

    /// @brief Pin configuration in use by this module.
    const MIRRAPins pins;
    /// @brief RTC module in use by this module.
    PCF2129_RTC rtc;
    /// @brief LoRa module in use by this module.
    LoRaModule lora;

    CommandEntry commandEntry;
};
};

#endif
