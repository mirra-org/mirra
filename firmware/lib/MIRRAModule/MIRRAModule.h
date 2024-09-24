#ifndef __MIRRAMODULE_H__
#define __MIRRAMODULE_H__

#include "Commands.h"
#include "FS.h"
#include "PCF2129_RTC.h"
#include "Pins.h"
#include "Protocol.h"

namespace mirra
{

/// @brief A base class for MIRRA modules to inherit from, which implements common functionality.
class MIRRAModule
{
public:
    /// @brief Configures the ESP32's pins, starts basic communication primitives (UART, I2C) and
    /// mounts the filesystem. Must be called before initialisation of the MIRRAModule.
    static void prepare();

protected:
    /// @brief Initialises the MIRRAModule, RTC, LoRa and logging modules.
    MIRRAModule();

    MIRRAModule(const MIRRAModule&) = delete;
    MIRRAModule& operator=(const MIRRAModule&) = delete;
    ~MIRRAModule() = default;

    struct Commands : CommonCommands
    {
        /// @brief Change the log level.
        /// @param arg String describing the new log level. ("ERROR", "INFO" or "DEBUG")
        CommandCode setLogLevel(const char* arg);
        /// @brief Prints the stored logs to the serial output.
        CommandCode printLogs();
        /// @brief Prints all stored data entries to the serial output in human readable format.
        CommandCode printData();
        /// @brief Prints all stored data entries to the serial output as a hex dump.
        CommandCode printDataRaw();
        /// @brief Formats the module, clearing the entire NVS and filesystem and restarts the
        /// module (effectively a hard reset).
        CommandCode format();
        /// @brief Spams the logs with a predetermined message ("abcdefghijklmnopqrstuvwxyz")
        /// @param count Amount of times the message is logged.
        CommandCode spam(size_t count);

        static constexpr auto getCommands()
        {
            return std::tuple_cat(
                CommonCommands::getCommands(),
                std::make_tuple(
                    CommandAliasesPair(&Commands::setLogLevel, "setlog", "setloglevel"),
                    CommandAliasesPair(&Commands::printLogs, "printlog", "printlogs",
                                       "printlogfile"),
                    CommandAliasesPair(&Commands::printData, "printdata", "printdatafile"),
                    CommandAliasesPair(&Commands::printDataRaw, "printdataraw", "printdatahex"),
                    CommandAliasesPair(&Commands::format, "format"),
                    CommandAliasesPair(&Commands::spam, "spam")));
        }
    };

    class SensorFile final : fs::FIFOFile
    {
        fs::NVS::Value<size_t> reader;

        size_t cutTail(size_t cutSize);

    public:
        struct DataEntry
        {
            struct Flags
            {
                uint8_t nValues : 7;
                bool uploaded : 1;
            };
            using SensorValueArray = comm::MessageBody<comm::DATA>::SensorValueArray;

            comm::Address source;
            uint32_t time;
            Flags flags;
            SensorValueArray values;

            static constexpr size_t flagsPosition = sizeof(source) + sizeof(time);

            static constexpr size_t getSize(Flags flags)
            {
                return sizeof(source) + sizeof(time) + sizeof(flags) +
                       flags.nValues * sizeof(SensorValue);
            }
            constexpr size_t getSize() const { return getSize(flags); }
        } __attribute__((packed));

        SensorFile();

        using FIFOFile::getMaxSize;
        using FIFOFile::getSize;

        using FIFOFile::read;

        class Iterator
        {
            size_t address;
            const SensorFile* file;
            Iterator(size_t address, const SensorFile* file) : address{address}, file{file} {}

        public:
            Iterator& operator++();
            bool operator!=(const Iterator& other) const { return this->address != other.address; }
            DataEntry operator*() const { return file->read<DataEntry>(address); }

            friend class SensorFile;
        };

        Iterator begin() const { return Iterator(0, this); };
        Iterator end() const { return Iterator(getSize(), this); };
        std::optional<size_t> getUnuploadedAddress(size_t index);
        std::optional<DataEntry> getUnuploaded(size_t index);
        bool isLast(size_t index);

        void push(const comm::Message<comm::DATA>& message);
        void push(const DataEntry& entry) { FIFOFile::push(&entry, entry.getSize()); }
        void setUploaded();

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
    static void end();

    /// @brief RTC module in use by this module.
    PCF2129_RTC rtc;

    /// @brief This module's MAC address.
    comm::MACAddress mac;

    CommandEntry commandEntry;
};
}

#endif
