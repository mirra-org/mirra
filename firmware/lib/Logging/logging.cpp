#include "logging.h"

using namespace mirra;

Log& Log::getInstance()
{
    static Log log{};
    return log;
}

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
            cutSize += std::distance(searchBuffer, found) + 1;
            return FIFOFile::cutTail(cutSize);
        }
    }
}

void Log::close()
{
    getInstance().~Log();
}