#ifndef __MIRRA_LOGGING_H__
#define __MIRRA_LOGGING_H__

#include "../MIRRAFS/FS.h"
#include <HardwareSerial.h>
#include <type_traits>

namespace mirra
{
class Log
{
public:
    enum class Level : uint8_t
    {
        DEBUG,
        INFO,
        ERROR
    };

private:
    Log() = default;
    Log(const Log&) = delete;
    Log(Log&&) = delete;
    Log& operator=(const Log&) = delete;
    Log& operator=(Log&&) = delete;

    class File final : fs::FIFOFile
    {
        size_t cutTail(size_t cutSize);

    public:
        File() : FIFOFile("logs") {}
        using FIFOFile::getSize;
        using FIFOFile::push;
        using FIFOFile::read;
    };

    /// @brief Currently loaded logging file.
    File file;

    /// @brief Buffer in which the final string is constructed and printed from.
    char buffer[256]{0};
    /// @brief Prints the preamble portion of the log line.
    /// @tparam level The level displayed in the preamble.
    /// @param time The time displayed in the preamble.
    /// @return The length of the preamble that was printed.
    template <Log::Level level> size_t printPreamble(const tm& time);
    /// @return String conversion from a level.
    static constexpr std::string_view levelToString(Level level);
    /// @brief Prints to the logging buffer and forwards to (if enabled) the output serial and
    /// logfile.
    /// @tparam level Log level of printed message.
    template <Log::Level level, class... Ts> void print(Ts&&... args);

public:
    /// @brief Serial to print to.
    HardwareSerial* serial{nullptr};
    /// @brief Logging level of the logging module. Messages below this level will not be printed.
    Level level{Level::INFO};
    /// @brief Whether logging to file is enabled.
    bool fileEnabled{false};

    /// @brief Singleton global log object
    static Log log;
    template <class... Ts> static void debug(Ts... args) { log.print<Level::DEBUG>(args...); }
    template <class... Ts> static void info(Ts... args) { log.print<Level::INFO>(args...); }
    template <class... Ts> static void error(Ts... args) { log.print<Level::ERROR>(args...); }

    /// @brief Initialises the logging module.
    static void init();
    /// @brief Closes the logging module, releasing the logging file.
    static void end();

    ~Log() = default;
};

/// @return A format specifier string matched to the type argument.
template <class T> constexpr std::string_view typeToFormatSpecifier();
/// @return A format string matched to the given type arguments.
template <class... Ts> constexpr auto createFormatString();
/// @brief Type-safe variadic print function. Uses compile-time format string instantiation to
/// ensure safety in using printf.
/// @param buffer Buffer to which to print.
/// @param max Max number of characters to print to buffer.
template <class... Ts> size_t printv(char* buffer, size_t max, Ts&&... args);

#include "./logging.tpp"
}

#endif