#include "Commands.h"

#include "FS.h"
#include "logging.h"
#include <Arduino.h>

using namespace mirra;

CommandEntry::CommandEntry(uint8_t checkPin, bool invert)
    : commandPhaseFlag{invert != static_cast<bool>(digitalRead(checkPin))}
{}

std::optional<std::array<char, CommandParser::lineMaxLength>> CommandParser::readLine()
{
    uint64_t timeout;
    uint8_t length{0};
    std::array<char, lineMaxLength> buffer{0};
    while (length < lineMaxLength - 1)
    {
        timeout = static_cast<uint64_t>(esp_timer_get_time()) + (UART_PHASE_TIMEOUT * 1000 * 1000);
        while (!Serial.available())
        {
            if (esp_timer_get_time() >= timeout)
                return std::nullopt;
        }
        char c = Serial.read();
        Serial.print(c);
        if (c == '\r' || c == '\n')
        {
            if (Serial.available()) // on Windows: both CR and LF
                Serial.print(static_cast<char>(Serial.read()));
            break;
        }
        else if (c == '\b') // backspace
        {
            if (length == 0)
                continue;
            Serial.print(' ');
            Serial.print('\b');
            length--;
            buffer[length] = '\0';
        }
        else
        {
            buffer[length] = c;
            length++;
            if (length == lineMaxLength - 1)
                break;
        }
    };
    return std::make_optional(buffer);
}

CommandCode CommonCommands::printLogs()
{
    static constexpr size_t bufferSize{256};
    char buffer[bufferSize];
    size_t cursor{0};
    Serial.printf("Logs: %u out of %u KB.\n", Log::log.file.getSize() / 1024,
                  Log::log.file.getMaxSize() / 1024);
    while (cursor < Log::log.file.getSize())
    {
        Log::log.file.read(cursor, buffer, bufferSize);
        Serial.write(buffer, std::min(bufferSize, Log::log.file.getSize() - cursor));
        cursor += bufferSize;
    }
    Serial.print('\n');
    return COMMAND_SUCCESS;
}

CommandCode CommonCommands::echo(const char* arg)
{
    Serial.print(arg);
    Serial.print('\n');
    return COMMAND_SUCCESS;
}

CommandCode CommonCommands::echoNum(int arg)
{
    Serial.print(arg);
    Serial.print('\n');
    return COMMAND_SUCCESS;
}

CommandCode CommonCommands::exit()
{
    return COMMAND_EXIT;
}