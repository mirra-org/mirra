#ifndef __MIRRA_FS_H__
#define __MIRRA_FS_H__

#include <cstring>
#include <esp_partition.h>
#include <memory>
#include <nvs.h>
#include <nvs_flash.h>
#include <optional>
#include <utility>

namespace mirra::fs
{
class NVS
{
private:
    nvs_handle_t handle;
    char name[NVS_KEY_NAME_MAX_SIZE];

    template <class T> std::optional<T> get(const char* key) const;
    template <class T> std::pair<esp_err_t, T> get_integral(const char* key) const;
    esp_err_t get_str(const char* key, char* buffer, size_t size) const;
    esp_err_t get_blob(const char* key, void* buffer, size_t size) const;

    template <class T> void set(const char* key, const T& value);
    template <class T> esp_err_t set_integral(const char* key, T value);
    esp_err_t set_str(const char* key, const char* value);
    esp_err_t set_blob(const char* key, const void* value, size_t size);

public:
    NVS(const char* name);
    ~NVS();

    void commit();

    template <class T> class Value
    {
        const char* key;
        NVS* nvs;
        T cachedValue;

        Value(const char* key, NVS* nvs) : key{key}, nvs{nvs}, cachedValue{nvs->get<T>(key).value()}
        {}
        Value(const char* key, NVS* nvs, const T& defaultValue)
            : key{key}, nvs{nvs}, cachedValue{nvs->get<T>(key).value_or(defaultValue)}
        {}

    public:
        Value(const Value&) = delete;
        Value(Value&&) = delete;
        Value& operator=(Value&&) = delete;

        ~Value() { commit(); };

        void commit() { nvs->set<T>(key, cachedValue); }
        T& operator=(const T& other) { return cachedValue = other; }
        T& operator=(T&& other) { return cachedValue = std::move(other); }
        operator T() const { return cachedValue; }
        operator T&() { return cachedValue; }
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
    template <class T> Value<T> getValue(const char* key) { return Value<T>(key, this); }
    template <class T> Value<T> getValue(const char* key, const T& defaultValue)
    {
        return Value<T>(key, this, defaultValue);
    }

    void eraseKey(const char* key);
    template <class T> void eraseValue(const Value<T>& value) { return eraseKey(value->key); }

    Iterator begin() const { return Iterator(name); };
    Iterator end() const { return Iterator(nullptr); };

    static void init();
};

class Partition
{
    static constexpr size_t partitionNameMaxSize = 16;
    static constexpr size_t sectorSize = 4096;
    static constexpr size_t toSectorAddress(size_t address);

    const esp_partition_t* part;
    char name[partitionNameMaxSize];
    size_t maxSize;
    std::unique_ptr<std::array<uint8_t, sectorSize>> sectorBuffer;
    size_t sectorAddress;
    bool sectorDirty;

    bool inSector(size_t address) const;
    void readSector(size_t sectorAddress);
    void writeSector();

protected:
    Partition(const char* name);
    void loadFirstSector(size_t address);

public:
    Partition(const Partition&) = delete;
    Partition(Partition&& other) = default;
    Partition& operator=(const Partition&) = delete;
    Partition& operator=(Partition&&) = default;
    ~Partition();

    size_t getMaxSize() const { return maxSize; };

    const char* getName() { return name; };
    void read(size_t address, void* buffer, size_t size) const;
    template <class T> T read(size_t address) const;

    void write(size_t address, const void* buffer, size_t size);
    template <class T> void write(size_t address, const T& value);

    void flush();
};
class FIFOFile : protected Partition
{
protected:
    NVS nvs;

private:
    NVS::Value<size_t> head;
    NVS::Value<size_t> size;
    NVS::Value<size_t> tail;

protected:
    FIFOFile(const char* name);
    /// @brief Cuts the beginning of the tail to free up space: how this cutting is implemented
    /// may be overriden.
    /// @param cutSize Minimal required size of cut in bytes.
    /// @return Total amount of bytes cut.
    virtual size_t cutTail(size_t cutSize);

public:
    FIFOFile(FIFOFile&&) = default;
    FIFOFile& operator=(FIFOFile&&) = default;
    virtual ~FIFOFile() {};

    size_t getSize() const { return size; }
    size_t freeSpace() const;

    using Partition::getName;

    void read(size_t address, void* buffer, size_t size) const;
    template <class T> T read(size_t address) const;

    void push(const void* buffer, size_t size);
    template <class T> void push(const T& value);

    void write(size_t address, const void* buffer, size_t size);
    template <class T> void write(size_t address, const T& value);

    void flush();
};

#include "./FS.tpp"
}

#endif
