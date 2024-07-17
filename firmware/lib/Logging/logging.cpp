#include "logging.h"

using namespace mirra;

Log Log::log{};

size_t Log::File::cutTail(size_t cutSize)
{
    static constexpr size_t searchSize{128};
    uint8_t searchBuffer[searchSize];
    while (true)
    {
        read(cutSize, &searchBuffer, searchSize);
        uint8_t* found = reinterpret_cast<uint8_t*>(std::memchr(searchBuffer, '\n', searchSize));
        if (found == nullptr)
        {
            cutSize += searchSize;
        }
        else
        {
            cutSize += std::distance(searchBuffer, found);
            break;
        }
    }
    return cutSize;
}

void Log::init() { Log::log.file = Log::File(); }

void Log::end() { Log::log.~Log(); }
