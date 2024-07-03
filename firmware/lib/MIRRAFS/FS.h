#ifndef __MIRRA_FS_H__
#define __MIRRA_FS_H__

#include <esp_partition.h>
#include <nvs.h>
#include <nvs_flash.h>
#include <string_view>
#include <type_traits>

namespace mirra::fs
{
    class NVS
    {
    private:
        nvs_handle_t handle;
        ~NVS();

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

        void set_u8(const char* key, const uint8_t value);
        void set_u16(const char* key, const uint16_t value);
        void set_u32(const char* key, const uint32_t value);
        void set_u64(const char* key, const uint64_t value);

        void set_i8(const char* key, const int8_t value);
        void set_i16(const char* key, const int16_t value);
        void set_i32(const char* key, const int32_t value);
        void set_i64(const char* key, const int64_t value);

        void set_str(const char* key, const char* value);
        void set_blob(const char* key, const void* value, size_t size);

    public:
        template <class T> class NVSValue
        {
            const char* key;
            const NVS& nvs;

            NVSValue(const char* key, const NVS& nvs) : key{key}, nvs{nvs} {}
            operator const T&() const { return nvs.get<T>(key); }
        };

        NVS(const char* name);
        template <class T> T get(const char* key) const;
        template <class T> void set(const char* key, T&& value);

        template <class T> NVSValue<T>& operator[](const char* key) { return NVSValue(key, this); }

        static void init();
    };

    class File
    {
        bool stale{false};

    public:
        void commit();
        void erase();

        File(const char* name);
        ~File();
    };
#include <FS.tpp>
}

#endif