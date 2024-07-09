#include "logging.h"

using namespace mirra;

Log Log::log{};

constexpr std::string_view Log::levelToString(Level level)
{
    switch (level)
    {
    case Level::ERROR:
        return "ERROR: ";
    case Level::INFO:
        return "INFO: ";
    case Level::DEBUG:
        return "DEBUG: ";
    }
    return "NONE: ";
}

size_t Log::File::cutTail(size_t cutSize) {}

void Log::init() { Log::log.file = Log::File(); }

void Log::close() { Log::log.~Log(); }
