#ifndef __COMMANDS_T__
#define __COMMANDS_T__
#include "Commands.h"

#include <Arduino.h>
#include <charconv>

template <class C>
typename std::enable_if_t<std::is_base_of_v<CommonCommands, C>, void>
CommandEntry::prompt(C&& commands)
{
    if (commandPhaseFlag)
        CommandParser().start<C>(std::forward<C>(commands));
    else
        Serial.println("Command phase flag not set. Command phase will not start.");
    commandPhaseFlag = false;
}

template <class N>
typename std::enable_if_t<std::is_arithmetic_v<N>, std::optional<N>>
CommandParser::parseNum(const char* buffer)
{
    N arith;
    if (std::from_chars(buffer, &buffer[strlen(buffer)], arith).ec == std::errc())
        return arith;
    else
        return std::nullopt;
}

template <class N>
typename std::enable_if_t<std::is_arithmetic_v<N>, std::optional<N>> CommandParser::readNum()
{
    auto line{readLine()};
    if (!line)
        return std::nullopt;
    return parseNum<N>(line->data());
}

template <class T>
typename std::enable_if_t<std::is_arithmetic_v<T> || std::is_same_v<std::decay_t<T>, char*>, bool>
CommandParser::editValue(T& value, bool skipWhenEmpty)
{
    while (true)
    {
        auto buffer{readLine()};
        if (!buffer)
            return false;
        if (skipWhenEmpty && strlen(buffer->data()) == 0)
            return true;
        if constexpr (std::is_same_v<std::decay_t<T>, char*>)
        {
            if constexpr (std::is_array_v<T>)
            {
                strncpy(value, buffer->data(), std::extent_v<T>);
            }
            else
            {
                strncpy(value, buffer->data(), strlen(value));
            }
            return true;
        }
        else
        {
            auto newValue{parseNum<T>(buffer->data())};
            if (newValue)
            {
                value = *newValue;
                return true;
            }
            else
            {
                Serial.printf("Could not parse input '%s' to valid numeric type. Please retry: \n",
                              buffer->data());
            }
        }
    }
}

template <class C>
typename std::enable_if_t<std::is_base_of_v<CommonCommands, C>, void>
CommandParser::start(C&& commands)
{
    Serial.println("COMMAND PHASE");
    while (true)
    {
        auto buffer{CommandParser::readLine()};
        if (!buffer)
        {
            Serial.println("Command phase timeout. Exiting...");
            return;
        }
        switch (parseLine(buffer->data(), std::forward<C>(commands)))
        {
        case COMMAND_ERROR:
            Serial.printf("Command '%s' encountered a fatal error.\n", buffer->data());
            break;
        case COMMAND_TIMEOUT:
            Serial.printf("Command '%s' timed out.\n", buffer->data());
            break;
        case COMMAND_EXIT:
            Serial.println("Exiting command phase...");
            break;
        default:
            break;
        }
    }
}

template <class Tup, size_t I = 0>
CommandCode CommandParser::parseArgs(Tup& argsTuple,
                                     const std::array<char*, std::tuple_size_v<Tup>>& buffers)
{
    if constexpr (I >= std::tuple_size_v<Tup>)
    {
        return COMMAND_SUCCESS;
    }
    else
    {
        auto& arg{std::get<I>(argsTuple)};
        using argType = std::remove_cv_t<std::remove_reference_t<decltype(arg)>>;
        const char* buffer{buffers[I]};
        if constexpr (std::is_same_v<argType, char*> || std::is_same_v<argType, const char*>)
        {
            arg = buffer;
        }
        else
        {
            auto parsed = parseNum<argType>(buffer);
            if (!parsed)
            {
                Serial.printf("Argument '%u':'%s' could not be parsed.\n", I, buffer);
                return COMMAND_ERROR;
            }
            arg = *parsed;
        }
        return parseArgs<Tup, I + 1>(argsTuple, buffers);
    }
}

template <class C, size_t I = 0> CommandCode CommandParser::parseLine(char* line, C&& commands)
{
    char* command{line};
    if constexpr (I == 0)
    {
        command = strtok(line, " \r\n");
        if (command == nullptr) // when no command entered, simply loop again
            return COMMAND_NOT_FOUND;
    }
    constexpr auto commandsTuple{C::getCommands()};
    if constexpr (I >= std::tuple_size_v<decltype(commandsTuple)>)
    {
        Serial.printf("Command '%s' not found.\n", command);
        return COMMAND_NOT_FOUND;
    }
    else
    {
        constexpr auto pair{std::get<I>(commandsTuple)};
        constexpr auto aliases{pair.getAliases()};
        for (const char* alias : aliases)
            if (strcmp(command, alias) == 0)
            {
                constexpr auto function{pair.getCommand()};
                typename decltype(pair)::ArgsTuple commandArgs;
                std::array<char*, pair.getCommandArgCount()> commandArgsBuffer;
                for (char*& commandArgBuffer : commandArgsBuffer)
                {
                    commandArgBuffer = strtok(nullptr, " \r\n");
                    if (commandArgBuffer == nullptr)
                    {
                        Serial.printf("Command '%s' expects %u argument(s) but received fewer.\n",
                                      alias, pair.getCommandArgCount());
                        return COMMAND_NOT_FOUND;
                    }
                }
                if (parseArgs(commandArgs, commandArgsBuffer) != COMMAND_SUCCESS)
                {
                    return COMMAND_ERROR;
                }
                // implement argument parsing here
                return std::apply(function,
                                  std::tuple_cat(std::forward_as_tuple(commands), commandArgs));
            }

        return parseLine<C, I + 1>(command, std::forward<C>(commands));
    }
}
#endif