#ifndef __COMMANDS_H__
#define __COMMANDS_H__

#include <array>
#include <optional>
#include <tuple>

#define UART_PHASE_TIMEOUT                                                                         \
    (1 * 60) // s, length of UART inactivity required to automatically exit command phase

enum CommandCode : uint8_t
{
    COMMAND_NOT_FOUND,
    COMMAND_ERROR,
    COMMAND_TIMEOUT,
    COMMAND_SUCCESS,
    COMMAND_EXIT
};

template <size_t nAliases, class C, class... CommandArgs>
struct CommandAliasesPair
    : private std::pair<CommandCode (C::*)(CommandArgs...), std::array<const char*, nAliases>>
{
    using AliasesArray = std::array<const char*, nAliases>;
    using Command = CommandCode (C::*)(CommandArgs...);
    using ArgsTuple = std::tuple<CommandArgs...>;
    using Pair = std::pair<Command, AliasesArray>;

    template <class... Aliases>
    constexpr CommandAliasesPair(Command&& f, Aliases&&... aliases)
        : Pair(std::forward<Command>(f), {{std::forward<Aliases>(aliases)...}})
    {}

    constexpr Command getCommand() const { return std::get<0>(*this); }
    constexpr AliasesArray getAliases() const { return std::get<1>(*this); }
    constexpr size_t getCommandArgCount() const { return sizeof...(CommandArgs); }
};

template <class... Aliases, class C, class... CommandArgs>
CommandAliasesPair(CommandCode (C::*&& f)(CommandArgs...), Aliases&&... aliases)
    -> CommandAliasesPair<sizeof...(Aliases), C, CommandArgs...>;

struct CommonCommands
{
    /// @brief Echoes the string argument to the UART output. Used for testing CLI functionality.
    /// @param arg The string argument to echo.
    CommandCode echo(const char* arg);
    /// @brief Echoes the integer argument to the UART output. Used for testing CLI functionality.
    /// @param arg The integer argument to echo.
    CommandCode echoNum(int arg);

    /// @brief Exits command mode.
    CommandCode exit();

    static constexpr auto getCommands()
    {
        return std::make_tuple(CommandAliasesPair(&CommonCommands::echo, "echo"),
                               CommandAliasesPair(&CommonCommands::echoNum, "echonum"),
                               CommandAliasesPair(&CommonCommands::exit, "exit", "close"));
    };
};

/// @brief Class responsible for entry into the commands system loop set by CommandParser.
class CommandEntry
{
    /// @brief Flag that determines if command phase should be entered when prompted.
    bool commandPhaseFlag{false};

public:
    /// @brief Creates the commands entry subsystem.
    /// @param flag Whether to sature the commandPhaseFlag or not.
    CommandEntry(bool flag = false) : commandPhaseFlag{flag} {}
    /// @brief Creates the commands entry subsystem and saturates its commandPhaseFlag.
    /// @param checkPin Pin to check for setting the commandPhaseFlag.
    /// @param invert Whether to invert the value of the read pin.
    CommandEntry(uint8_t checkPin, bool invert = false);
    /// @brief Checks the commandPhaseFlag and enters the command phase if it is set.
    /// @tparam C Commands set to be used for this command phase.
    template <class C>
    typename std::enable_if_t<std::is_base_of_v<CommonCommands, C>, void> prompt(C&& commands);
    // @brief Forcibly sets the commandPhaseFlag to true.
    void setFlag() { commandPhaseFlag = true; };
};

class CommandParser
{
    /// @brief Maximum line length in characters when entering commands.
    static constexpr size_t lineMaxLength{256};
    template <class Tup, size_t I = 0>
    CommandCode parseArgs(Tup& argsTuple, const std::array<char*, std::tuple_size_v<Tup>>& buffers);
    /// @brief Reads the command, calls the correct function with appropriate arguments.
    /// @tparam C Commands set to be used for this command phase.
    /// @param line Pointer to char buffer holding the command.
    /// @return A command code desribing the result of the execution of the command.
    template <class C, size_t I = 0> CommandCode parseLine(char* line, C&& commands);

public:
    /// @brief Reads a line from the UART input stream. Helper function to be used during the
    /// command phase.
    /// @return An array representing a command sequence entered by the user. Disengaged if this
    /// function times out.
    static std::optional<std::array<char, lineMaxLength>> readLine();

    template <class N>
    static typename std::enable_if_t<std::is_arithmetic_v<N>, std::optional<N>>
    parseNum(const char* buffer);
    template <class N>
    static typename std::enable_if_t<std::is_arithmetic_v<N>, std::optional<N>> readNum();
    /// @brief Helper function to facilitate the editing of a value (numeric or string) for the
    /// user.
    /// @param value Value to edit.
    /// @param skipWhenEmpty Whether to skip the editing of the value when the user enters an empty
    /// buffer (i.e. just presses Enter).
    template <class T>
    static
        typename std::enable_if_t<std::is_arithmetic_v<T> || std::is_same_v<std::decay_t<T>, char*>,
                                  bool>
        editValue(T& value, bool skipWhenEmpty = true);
    /// @brief Initialises the command loop, exiting when the appropriate command code is returned
    /// by processCommands.
    /// @tparam C Commands set to be used for this command phase.
    template <class C>
    typename std::enable_if_t<std::is_base_of_v<CommonCommands, C>, void> start(C&& commands);
};

#include "Commands.tpp"
#endif