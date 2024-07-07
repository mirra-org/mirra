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
    Log(Log&&) = default;
    Log& operator=(const Log&) = delete;
    Log& operator=(Log&&) = default;

    class File final : public fs::FIFOFile
    {
        size_t cutTail(size_t cutSize);

    public:
        File() : FIFOFile("logs") {}
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
    /// @return A format specifier string matched to the type argument.
    template <class T> static constexpr std::string_view typeToFormatSpecifier();
    template <class T> static constexpr std::string_view rawTypeToFormatSpecifier();
    /// @return A format string matched to the given type arguments.
    template <class... Ts> static constexpr auto createFormatString();
    /// @brief Type-safe variadic print function. Uses compile-time format string instantiation to
    /// ensure safety in using printf.
    /// @param buffer Buffer to which to print.
    /// @param max Max number of characters to print to buffer.
    template <class... Ts> size_t printv(char* buffer, size_t max, Ts&&... args);
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
    static void close();

    ~Log() = default;
};

template <Log::Level level> size_t Log::printPreamble(const tm& time)
{
    constexpr size_t timeLength{sizeof("[0000-00-00 00:00:00]")};
    strftime(buffer, timeLength, "[%F %T]", &time);
    constexpr std::string_view levelString{levelToString(level)};
    strcpy(&buffer[timeLength - 1], levelString.cbegin());
    return timeLength + levelString.size() - 1;
}

// TODO: Clean up the type to format specifier compile-time functionality with proper SFINAE. This
// here works, but it's messy, requires two functions, and is unruly to extend.

template <class T> constexpr std::string_view Log::typeToFormatSpecifier()
{
    using rawType = std::decay_t<T>;
    if constexpr (std::is_enum_v<rawType>)
    {
        using rawType = std::underlying_type_t<rawType>;
    }
    if constexpr (std::is_same_v<std::make_unsigned_t<rawType>, unsigned char>)
    {
        if constexpr (std::is_signed_v<rawType>)
        {
            return rawTypeToFormatSpecifier<signed int>();
        }
        else
        {
            return rawTypeToFormatSpecifier<unsigned int>();
        }
    }
    else
    {
        return rawTypeToFormatSpecifier<rawType>();
    }
}

template <> constexpr std::string_view Log::rawTypeToFormatSpecifier<const char*>() { return "%s"; }
template <> constexpr std::string_view Log::rawTypeToFormatSpecifier<char*>() { return "%s"; }
template <> constexpr std::string_view Log::rawTypeToFormatSpecifier<signed int>() { return "%i"; }
template <> constexpr std::string_view Log::rawTypeToFormatSpecifier<unsigned int>()
{
    return "%u";
}
template <> constexpr std::string_view Log::rawTypeToFormatSpecifier<long signed int>()
{
    return "%i";
}
template <> constexpr std::string_view Log::rawTypeToFormatSpecifier<signed char>() { return "%i"; }
template <> constexpr std::string_view Log::rawTypeToFormatSpecifier<unsigned char>()
{
    return "%u";
}
template <> constexpr std::string_view Log::rawTypeToFormatSpecifier<float>() { return "%f"; }
template <> constexpr std::string_view Log::rawTypeToFormatSpecifier<char>() { return "%c"; }

template <class... Ts> constexpr auto Log::createFormatString()
{
    std::array<char, (typeToFormatSpecifier<Ts>().size() + ... + 1)> fmt{0};
    size_t i{0};
    for (const auto& s : std::array{typeToFormatSpecifier<Ts>()...})
    {
        for (auto c : s)
            fmt[i++] = c;
    }
    return fmt;
}

template <class... Ts> size_t Log::printv(char* buffer, size_t max, Ts&&... args)
{
    constexpr auto fmt{createFormatString<Ts...>()};
    return snprintf(buffer, max, fmt.data(), std::forward<Ts>(args)...);
}
template <Log::Level level, class... Ts> void Log::print(Ts&&... args)
{
    if (level < this->level)
        return;
    time_t ctime{time(nullptr)};
    tm* time{gmtime(&ctime)};
    size_t size{printPreamble<level>(*time)};
    size += printv(&buffer[size], sizeof(buffer) - size, std::forward<Ts>(args)...);
    buffer[size] = '\n';
    size++;
    file.push(buffer, size);
    serial->write(buffer, size);
}

}

#endif