#include "FS.h"

using namespace mirra::fs;

NVS::NVS(const char* name)
{
    if (nvs_open(name, NVS_READWRITE, &handle) != ESP_OK) {
        // log error
    };
    std::strncpy(this->name, name, NVS_KEY_NAME_MAX_SIZE);
}

NVS::~NVS()
{
    commit();
    nvs_close(handle);
}

void NVS::commit()
{
    nvs_commit(handle);
}
uint8_t NVS::get_u8(const char* key) const
{
    uint8_t value{0};
    if (nvs_get_u8(handle, key, &value) != ESP_OK)
        ; // log error
    return value;
}

uint16_t NVS::get_u16(const char* key) const
{
    uint16_t value{0};
    if (nvs_get_u16(handle, key, &value) != ESP_OK)
        ; // log error
    return value;
}

uint32_t NVS::get_u32(const char* key) const
{
    uint32_t value{0};
    if (nvs_get_u32(handle, key, &value) != ESP_OK)
        ; // log error
    return value;
}

uint64_t NVS::get_u64(const char* key) const
{
    uint64_t value{0};
    if (nvs_get_u64(handle, key, &value) != ESP_OK)
        ; // log error
    return value;
}

int8_t NVS::get_i8(const char* key) const
{
    int8_t value{0};
    if (nvs_get_i8(handle, key, &value) != ESP_OK)
        ; // log error
    return value;
}

int16_t NVS::get_i16(const char* key) const
{
    int16_t value{0};
    if (nvs_get_i16(handle, key, &value) != ESP_OK)
        ; // log error
    return value;
}

int32_t NVS::get_i32(const char* key) const
{
    int32_t value{0};
    if (nvs_get_i32(handle, key, &value) != ESP_OK)
        ; // log error
    return value;
}

int64_t NVS::get_i64(const char* key) const
{
    int64_t value{0};
    if (nvs_get_i64(handle, key, &value) != ESP_OK)
        ; // log error
    return value;
}

template <> uint8_t NVS::get(const char* key) const
{
    return get_u8(key);
}
template <> uint16_t NVS::get(const char* key) const
{
    return get_u16(key);
}
template <> uint32_t NVS::get(const char* key) const
{
    return get_u32(key);
}
template <> uint64_t NVS::get(const char* key) const
{
    return get_u64(key);
}

template <> int8_t NVS::get(const char* key) const
{
    return get_i8(key);
}
template <> int16_t NVS::get(const char* key) const
{
    return get_i16(key);
}
template <> int32_t NVS::get(const char* key) const
{
    return get_i32(key);
}
template <> int64_t NVS::get(const char* key) const
{
    return get_i64(key);
}

void NVS::get_str(const char* key, char* buffer, size_t size) const
{
    if (nvs_get_str(handle, key, buffer, &size) != ESP_OK)
        ; // log error
}

void NVS::get_blob(const char* key, void* buffer, size_t size) const
{
    if (nvs_get_blob(handle, key, buffer, &size) != ESP_OK)
        ; // log error
}

void NVS::set_u8(const char* key, uint8_t value)
{
    if (nvs_set_u8(handle, key, value) != ESP_OK)
        ; // log error
}

void NVS::set_u16(const char* key, uint16_t value)
{
    if (nvs_set_u16(handle, key, value) != ESP_OK)
        ; // log error
}

void NVS::set_u32(const char* key, uint32_t value)
{
    if (nvs_set_u32(handle, key, value) != ESP_OK)
        ; // log error
}

void NVS::set_u64(const char* key, uint64_t value)
{
    if (nvs_set_u64(handle, key, value) != ESP_OK)
        ; // log error
}

void NVS::set_i8(const char* key, int8_t value)
{
    if (nvs_set_i8(handle, key, value) != ESP_OK)
        ; // log error
}

void NVS::set_i16(const char* key, int16_t value)
{
    if (nvs_set_i16(handle, key, value) != ESP_OK)
        ; // log error
}

void NVS::set_i32(const char* key, int32_t value)
{
    if (nvs_set_i32(handle, key, value) != ESP_OK)
        ; // log error
}

void NVS::set_i64(const char* key, int64_t value)
{
    if (nvs_set_i64(handle, key, value) != ESP_OK)
        ; // log error
}

void NVS::set_str(const char* key, const char* buffer)
{
    if (nvs_set_str(handle, key, buffer) != ESP_OK)
        ; // log error
}

void NVS::set_blob(const char* key, const void* value, size_t size)
{
    if (nvs_set_blob(handle, key, value, size) != ESP_OK)
        ; // log error
}

template <> void NVS::set(const char* key, const uint8_t& value)
{
    set_u8(key, value);
}
template <> void NVS::set(const char* key, const uint16_t& value)
{
    set_u16(key, value);
}
template <> void NVS::set(const char* key, const uint32_t& value)
{
    set_u32(key, value);
}
template <> void NVS::set(const char* key, const uint64_t& value)
{
    set_u64(key, value);
}

template <> void NVS::set(const char* key, const int8_t& value)
{
    set_i8(key, value);
}
template <> void NVS::set(const char* key, const int16_t& value)
{
    set_i16(key, value);
}
template <> void NVS::set(const char* key, const int32_t& value)
{
    set_i32(key, value);
}
template <> void NVS::set(const char* key, const int64_t& value)
{
    set_i64(key, value);
}

template <> void NVS::set(const char* key, const char* const& value)
{
    set_str(key, value);
}

bool NVS::exists(const char* key) const
{
    int8_t dummy;
    esp_err_t err = nvs_get_i8(handle, key, &dummy);
    return (err == ESP_ERR_NVS_TYPE_MISMATCH) || (err == ESP_OK);
}

void NVS::init()
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        nvs_flash_erase();
        err = nvs_flash_init();
    }
    if (err != ESP_OK) {
        // log error
    }
}

Partition::Partition(const char* name)
    : part{esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_UNDEFINED,
                                    name)},
      sectorBuffer{new std::array<uint8_t, sectorSize>}, maxSize{part->size}
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
    return sectorAddress <= address && address < sectorAddress + sectorSize;
}

void Partition::readSector(size_t sectorAddress)
{
    flush();
    esp_err_t err = esp_partition_read(part, sectorAddress, sectorBuffer.get(), sectorSize);
    if (err != ESP_OK)
        ; // log error
    this->sectorAddress = sectorAddress;
}

void Partition::writeSector()
{
    esp_err_t err = esp_partition_erase_range(part, sectorAddress, sectorSize);
    if (err != ESP_OK)
        ; // log error
    esp_partition_write(part, sectorAddress, sectorBuffer.get(), sectorSize);
    if (err != ESP_OK)
        ; // log error
    sectorDirty = false;
}

void Partition::read(size_t address, void* buffer, size_t size) const
{
    // if address in sector buffer: read straight from it
    if (inSector(address)) {
        size_t toRead = std::min(sectorSize - (address - sectorAddress), size);
        std::memcpy(buffer, &sectorBuffer.get()[address - sectorAddress], toRead);
        address += toRead;
        buffer = static_cast<uint8_t*>(buffer) + toRead;
        size -= toRead;
    }
    // else (or if not all requested data in sector buffer), read straight from flash
    while (size > 0) {
        size_t toRead = std::min(maxSize - address, size);
        esp_partition_read(part, address, buffer, size);
        address = (address + toRead) % getMaxSize();
        buffer = static_cast<uint8_t*>(buffer) + toRead;
        size -= toRead;
    }
}

void Partition::write(size_t address, const void* buffer, size_t size)
{
    while (size > 0) {
        if (!inSector(address))
            readSector(toSectorAddress(address));

        size_t toWrite = std::min(sectorSize - (address - sectorAddress), size);
        std::memcpy(&sectorBuffer.get()[address - sectorAddress], buffer, toWrite);
        address = (address + toWrite) % getMaxSize();
        buffer = static_cast<const uint8_t*>(buffer) + toWrite;
        size -= toWrite;
        sectorDirty = true;
    }
}

void Partition::flush()
{
    if (sectorDirty) {
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
