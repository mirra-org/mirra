#include "FIFOFile.h"

using namespace mirra::fs;

FIFOFile::FIFOFile(const char* name) : File(name)
{
    NVS nvs{name};
    NVS::Value<size_t> head{nvs.getValue<size_t>("head")};
    if (!head.exists())
        head = this->head = 0;
    else
        this->head = head;

    NVS::Value<size_t> tail{nvs.getValue<size_t>("tail")};
    if (!tail.exists())
        tail = this->tail = 0;
    else
        this->tail = tail;
}

FIFOFile::~FIFOFile()
{
    NVS nvs{getName()};
    nvs.getValue<size_t>("head") = head;
    nvs.getValue<size_t>("tail") = tail;
}

size_t FIFOFile::size() const
{
    if (head >= tail)
    {
        return head - tail;
    }
    return part->size - (tail - head);
}

bool FIFOFile::inFile(size_t address) const
{
    if (head >= tail)
    {
        return address >= tail && address < head;
    }
    return !(address >= head && address < tail);
}

void FIFOFile::read(size_t address, void* buffer, size_t size) const
{
    size_t fileSize = this->size();
    if (address >= fileSize)
        return;
    File::read((tail + address) % part->size, buffer, std::min(size, fileSize - address));
}

void FIFOFile::write(size_t address, const void* buffer, size_t size)
{
    size_t fileSize = this->size();
    if (address >= fileSize)
        return;
    File::write((tail + address) % part->size, buffer, std::min(size, fileSize - address));
}

size_t FIFOFile::segmentor(size_t size)
{
    size_t oldHead = head;
    head = (head + size) % part->size;
    // has head caught up with tail? -> call segmentor to cleanly cut tail
    if ((oldHead <= tail && tail < head) || (tail <= oldHead))
        tail = (tail + size)
}