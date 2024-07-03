#include "FS.h"

using namespace mirra::fs;

NVS::NVS(const char* name)
{
    if (nvs_open(name, NVS_READWRITE, &handle) != ESP_OK)
    {
        // log error
    };
}

NVS::~NVS()
{
    nvs_commit(handle);
    nvs_close(handle);
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

void NVS::init()
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        nvs_flash_erase();
        err = nvs_flash_init() == ESP_OK;
    }
    if (err != ESP_OK)
    {
        // log error
    }
}

File::File(const char* name) { const esp_partition_t* part{esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_UNDEFINED, name)}; }