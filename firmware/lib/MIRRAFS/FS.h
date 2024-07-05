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
        template <class T> void set(const char* key, const T& value);

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
            Value& operator=(const T& other)
            {
                nvs.set(key, other);
                return *this;
            }
            Value& operator=(T&& other)
            {
                nvs.set(key, other);
                return *this;
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
            Iterator operator++()
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

        Iterator begin() { return Iterator(name); };
        Iterator end() { return Iterator(nullptr); };

        static void init();
    };

    class File
    {
    protected:
        const esp_partition_t* part;

    private:
        char* name;
        static constexpr size_t partitionNameMaxSize = 16;
        static constexpr size_t sectorSize = 4096;
        std::unique_ptr<uint8_t[sectorSize]> sectorBuffer{};
        size_t sectorAddress;
        bool sectorDirty;

        size_t toSectorAddress(size_t address) const;
        bool inSector(size_t address) const;
        void readSector(size_t sectorAddress);
        void writeSector();

    public:
        File(const char* name);
        ~File();

        const char* getName() { return name; };
        void read(size_t address, void* buffer, size_t size) const;
        template <class T> T read(size_t address) const;

        void write(size_t address, const void* buffer, size_t size);
        template <class T> void write(size_t address, const T& value);

        void flush();
    };
#include "FS.tpp"
}

#endif