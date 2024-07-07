#ifndef __MIRRA_FS_H__
#define __MIRRA_FS_H__

#include <cstring>
#include <esp_partition.h>
#include <memory>
#include <nvs.h>
#include <nvs_flash.h>
#include <utility>

namespace mirra::fs
{
class NVS
{
private:
    nvs_handle_t handle;
    char name[NVS_KEY_NAME_MAX_SIZE];

    uint8_t get_u8(const char* key) const;
    uint16_t get_u16(const char* key) const;
    uint32_t get_u32(const char* key) const;
    uint64_t get_u64(const char* key) const;

    int8_t get_i8(const char* key) const;
    int16_t get_i16(const char* key) const;
    int32_t get_i32(const char* key) const;
    int64_t get_i64(const char* key) const;

    void get_str(const char* key, char* buffer, size_t size) const;
    void get_blob(const char* key, void* buffer, size_t size) const;

    void set_u8(const char* key, uint8_t value);
    void set_u16(const char* key, uint16_t value);
    void set_u32(const char* key, uint32_t value);
    void set_u64(const char* key, uint64_t value);

    void set_i8(const char* key, int8_t value);
    void set_i16(const char* key, int16_t value);
    void set_i32(const char* key, int32_t value);
    void set_i64(const char* key, int64_t value);

    void set_str(const char* key, const char* value);
    void set_blob(const char* key, const void* value, size_t size);

    bool exists(const char* key) const;
    template <class T> T get(const char* key) const;
    template <class T> void set(const char* key, T&& value);

public:
    NVS(const char* name);
    ~NVS();

    template <class T> class Value
    {
        const char* key;
        NVS& nvs;

        Value(const char* key, NVS& nvs) : key{key}, nvs{nvs} {}
        Value(const Value& other) = delete;
        Value(Value&&) = delete;

    public:
        bool exists() const { return nvs.exists(key); }
        T getOrCreate(const T& value) { return exists() ? *this : *this = value; }
        const T& operator=(const T& other)
        {
            nvs.set(key, other);
            return other;
        }
        const T& operator=(T&& other)
        {
            nvs.set(key, std::move(other));
            return other;
        }
        operator T() const { return nvs.get<T>(key); }
        friend class NVS;
    };

    class Iterator
    {
        nvs_iterator_t nvsIterator;
        nvs_entry_info_t nvsInfo;

        Iterator(const char* name) : nvsIterator{nvs_entry_find("nvs", name, NVS_TYPE_ANY)} {}
        Iterator(nullptr_t) : nvsIterator{nullptr} {}

    public:
        Iterator& operator++()
        {
            nvsIterator = nvs_entry_next(nvsIterator);
            return *this;
        }
        bool operator!=(const Iterator& other) const { return nvsIterator != other.nvsIterator; }
        const char* operator*()
        {
            nvs_entry_info(nvsIterator, &nvsInfo);
            return nvsInfo.key;
        }
        ~Iterator() { nvs_release_iterator(nvsIterator); }
        friend class NVS;
    };
    template <class T> Value<T> getValue(const char* key) & { return Value<T>(key, *this); }
    template <class T> T getValue(const char* key) && { return Value<T>(key, *this); }

    Iterator begin() const { return Iterator(name); };
    Iterator end() const { return Iterator(nullptr); };

    static void init();
};

class Partition
{
    static constexpr size_t partitionNameMaxSize = 16;
    static constexpr size_t sectorSize = 4096;
    static constexpr size_t toSectorAddress(size_t address);

    char* name;
    std::unique_ptr<std::array<uint8_t, sectorSize>> sectorBuffer;
    size_t sectorAddress;
    bool sectorDirty;

    bool inSector(size_t address) const;
    void readSector(size_t sectorAddress);
    void writeSector();

protected:
    const esp_partition_t* part;
    Partition(const char* name);
    void loadFirstSector(size_t address);

public:
    Partition(const Partition&) = delete;
    Partition(Partition&& other) = default;
    Partition& operator=(const Partition&) = delete;
    Partition& operator=(Partition&&) = default;
    ~Partition();

    const char* getName() { return name; };
    void read(size_t address, void* buffer, size_t size) const;
    template <class T> T read(size_t address) const;

    void write(size_t address, const void* buffer, size_t size);
    template <class T> void write(size_t address, const T& value);

    void flush();
};
class FIFOFile : Partition
{
private:
    /// @brief Cuts the beginning of the tail to free up space: how this cutting is implemented
    /// may be overriden.
    /// @param cutSize Minimal required size of cut in bytes.
    /// @return Total amount of bytes cut.
    virtual size_t cutTail(size_t cutSize);
    size_t head;
    size_t size;

protected:
    FIFOFile(const char* name);
    size_t tail;

public:
    FIFOFile(FIFOFile&&) = default;
    FIFOFile& operator=(FIFOFile&&) = default;
    virtual ~FIFOFile();

    size_t getSize() const { return size; }
    size_t freeSpace() const;

    void read(size_t address, void* buffer, size_t size) const;
    template <class T> T read(size_t address) const;

    void push(const void* buffer, size_t size);
    template <class T> void push(const T& value);

    void write(size_t address, const void* buffer, size_t size);
    template <class T> void write(size_t address, const T& value);

    using Partition::flush;
};
#include "FS.tpp"
}

#endif