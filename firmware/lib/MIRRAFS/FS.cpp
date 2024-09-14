#include "FS.h"

using namespace mirra::fs;

NVS::NVS(const char* name)
{
    esp_err_t err = nvs_open(name, NVS_READWRITE, &handle);
    if (err != ESP_OK)
        printf("Error while opening NVS namespace '%s', code: %i\n", name, err);
    std::strncpy(this->name, name, NVS_KEY_NAME_MAX_SIZE);
}

NVS::~NVS()
{
    commit();
    nvs_close(handle);
}

void NVS::commit()
{
    esp_err_t err = nvs_commit(handle);
    if (err != ESP_OK)
        printf("Error while committing changes to NVS, code: %i\n", err);
}
template <> std::pair<esp_err_t, uint8_t> NVS::get_integral(const char* key) const
{
    uint8_t value{0};
    return std::make_pair(nvs_get_u8(handle, key, &value), value);
}

template <> std::pair<esp_err_t, uint16_t> NVS::get_integral(const char* key) const
{
    uint16_t value{0};
    return std::make_pair(nvs_get_u16(handle, key, &value), value);
}

template <> std::pair<esp_err_t, uint32_t> NVS::get_integral(const char* key) const
{
    uint32_t value{0};
    return std::make_pair(nvs_get_u32(handle, key, &value), value);
}

template <> std::pair<esp_err_t, uint64_t> NVS::get_integral(const char* key) const
{
    uint64_t value{0};
    return std::make_pair(nvs_get_u64(handle, key, &value), value);
}

template <> std::pair<esp_err_t, int8_t> NVS::get_integral(const char* key) const
{
    int8_t value{0};
    return std::make_pair(nvs_get_i8(handle, key, &value), value);
}

template <> std::pair<esp_err_t, int16_t> NVS::get_integral(const char* key) const
{
    int16_t value{0};
    return std::make_pair(nvs_get_i16(handle, key, &value), value);
}

template <> std::pair<esp_err_t, int32_t> NVS::get_integral(const char* key) const
{
    int32_t value{0};
    return std::make_pair(nvs_get_i32(handle, key, &value), value);
}

template <> std::pair<esp_err_t, int64_t> NVS::get_integral(const char* key) const
{
    int64_t value{0};
    return std::make_pair(nvs_get_i64(handle, key, &value), value);
}

esp_err_t NVS::get_str(const char* key, char* buffer, size_t size) const
{
    return nvs_get_str(handle, key, buffer, &size);
}

esp_err_t NVS::get_blob(const char* key, void* buffer, size_t size) const
{
    return nvs_get_blob(handle, key, buffer, &size);
}

template <> esp_err_t NVS::set_integral<uint8_t>(const char* key, uint8_t value)
{
    return nvs_set_u8(handle, key, value);
}
template <> esp_err_t NVS::set_integral<uint16_t>(const char* key, uint16_t value)
{
    return nvs_set_u16(handle, key, value);
}

template <> esp_err_t NVS::set_integral<uint32_t>(const char* key, uint32_t value)
{
    return nvs_set_u32(handle, key, value);
}

template <> esp_err_t NVS::set_integral<uint64_t>(const char* key, uint64_t value)
{
    return nvs_set_u64(handle, key, value);
}

template <> esp_err_t NVS::set_integral<int8_t>(const char* key, int8_t value)
{
    return nvs_set_i8(handle, key, value);
}
template <> esp_err_t NVS::set_integral<int16_t>(const char* key, int16_t value)
{
    return nvs_set_i16(handle, key, value);
}
template <> esp_err_t NVS::set_integral<int32_t>(const char* key, int32_t value)
{
    return nvs_set_i32(handle, key, value);
}
template <> esp_err_t NVS::set_integral<int64_t>(const char* key, int64_t value)
{
    return nvs_set_i64(handle, key, value);
}

esp_err_t NVS::set_str(const char* key, const char* value)
{
    return nvs_set_str(handle, key, value);
}

esp_err_t NVS::set_blob(const char* key, const void* value, size_t size)
{
    return nvs_set_blob(handle, key, value, size);
}

void NVS::eraseKey(const char* key)
{
    esp_err_t err;
    err = nvs_erase_key(handle, key);
    if (err != ESP_OK)
        printf("Error while erasing key '%s', code: %s\n", key, esp_err_to_name(err));
}

void NVS::init()
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        printf("NVS partition was truncated and needs to be erased.\n");
        nvs_flash_erase();
        err = nvs_flash_init();
    }
    if (err != ESP_OK)
        printf("Error while initialising NVS flash, code: %s\n", esp_err_to_name(err));
}

Partition::Partition(const char* name)
    : part{esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_UNDEFINED,
                                    name)},
      maxSize{part->size}, sectorBuffer{new std::array<uint8_t, sectorSize>}
{
    strncpy(this->name, name, partitionNameMaxSize);
}

Partition::~Partition()
{
    flush();
}

constexpr size_t Partition::toSectorAddress(size_t address)
{
    return (address / sectorSize) * sectorSize;
}

void Partition::loadFirstSector(size_t address)
{
    sectorDirty = false;
    readSector(toSectorAddress(address));
}

bool Partition::inSector(size_t address) const
{
    return (sectorAddress <= address) && (address < (sectorAddress + sectorSize));
}

void Partition::readSector(size_t sectorAddress)
{
    flush();
    esp_err_t err = esp_partition_read(part, sectorAddress, sectorBuffer.get(), sectorSize);
    if (err != ESP_OK)
    {
        printf("Error while reading sector %u from partition '%s', code: %s\n", sectorAddress,
               getName(), esp_err_to_name(err));
    }
    this->sectorAddress = sectorAddress;
}

void Partition::writeSector()
{
    esp_err_t err = esp_partition_erase_range(part, sectorAddress, sectorSize);
    if (err != ESP_OK)
        printf("Error while erasing sector %u from partition '%s', code: %s\n", sectorAddress,
               getName(), esp_err_to_name(err));
    err = esp_partition_write(part, sectorAddress, sectorBuffer.get(), sectorSize);
    if (err != ESP_OK)
        printf("Error while writing sector %u from partition '%s', code: %s\n", sectorAddress,
               getName(), esp_err_to_name(err));
    sectorDirty = false;
}

void Partition::read(size_t address, void* buffer, size_t size) const
{
    while (size > 0)
    {
        size_t toRead;
        if (inSector(address)) // if address in sector buffer: read straight from it
        {
            toRead = std::min(sectorSize - (address - sectorAddress), size);
            std::memcpy(buffer, &(*sectorBuffer)[address - sectorAddress], toRead);
        }
        else // else (or if not all requested data in sector buffer), read straight from flash
        {
            toRead = std::min((address < sectorAddress ? sectorAddress : maxSize) - address, size);
            esp_partition_read(part, address, buffer, size);
        }
        address = (address + toRead) % getMaxSize();
        buffer = static_cast<uint8_t*>(buffer) + toRead;
        size -= toRead;
    }
}

void Partition::write(size_t address, const void* buffer, size_t size)
{
    while (size > 0)
    {
        if (!inSector(address))
            readSector(toSectorAddress(address));

        size_t toWrite = std::min(sectorSize - (address - sectorAddress), size);
        std::memcpy(&(*sectorBuffer)[address - sectorAddress], buffer, toWrite);
        address = (address + toWrite) % getMaxSize();
        buffer = static_cast<const uint8_t*>(buffer) + toWrite;
        size -= toWrite;
        sectorDirty = true;
    }
}

void Partition::flush()
{
    if (sectorDirty)
    {
        writeSector();
    }
}

FIFOFile::FIFOFile(const char* name)
    : Partition(name), nvs{getName()}, head{nvs.getValue<size_t>("head", 0)},
      tail{nvs.getValue<size_t>("tail", 0)}, size{nvs.getValue<size_t>("size", 0)}
{
    loadFirstSector(head);
}

size_t FIFOFile::freeSpace() const
{
    return getMaxSize() - size;
}

void FIFOFile::read(size_t address, void* buffer, size_t size) const
{
    if (address >= this->size)
        return;
    Partition::read((tail + address) % getMaxSize(), buffer, std::min(size, this->size - address));
}

void FIFOFile::push(const void* buffer, size_t size)
{
    this->size = (this->size + size) - (freeSpace() < size ? cutTail(size - freeSpace()) : 0);
    Partition::write(head, buffer, size);
    head = (head + size) % getMaxSize();
}

void FIFOFile::write(size_t address, const void* buffer, size_t size)
{
    if (address >= this->size)
        return;
    Partition::write((tail + address) % getMaxSize(), buffer, std::min(size, this->size - address));
}

size_t FIFOFile::cutTail(size_t cutSize)
{
    tail = (tail + cutSize) % getMaxSize();
    return cutSize;
}

void FIFOFile::flush()
{
    head.commit();
    tail.commit();
    size.commit();
    nvs.commit();
    Partition::flush();
}
