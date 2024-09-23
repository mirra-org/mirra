#ifndef __MIRRA_FS_T__
#define __MIRRA_FS_T__

static_assert(std::is_integral_v<size_t>);

template <class T> std::optional<T> NVS::get(const char* key) const
{
    esp_err_t err;
    T value;
    if constexpr (std::is_integral_v<T>)
    {
        std::tie(err, value) = get_integral<T>(key);
    }
    else
    {
        err = get_blob(key, &value, sizeof(value));
    }
    if (err == ESP_OK)
        return value;
    if (err != ESP_ERR_NVS_NOT_FOUND)
        printf("Error while getting key '%s', code: %s\n", key, esp_err_to_name(err));
    return std::nullopt;
}

template <class T> void NVS::set(const char* key, const T& value)
{
    esp_err_t err;
    if constexpr (std::is_integral_v<T>)
    {
        err = set_integral<T>(key, value);
    }
    else
    {
        err = set_blob(key, &value, sizeof(T));
    }
    if (err != ESP_OK)
        printf("Error while setting key '%s', code: %s\n", key, esp_err_to_name(err));
}

template <class T> T Partition::read(size_t address) const
{
    T buffer;
    read(address, &buffer, sizeof(T));
    return buffer;
}

template <class T> void Partition::write(size_t address, const T& value)
{
    write(address, &value, sizeof(T));
}

template <class T> T FIFOFile::read(size_t address) const
{
    T buffer;
    read(address, &buffer, sizeof(T));
    return buffer;
}

template <class T> void FIFOFile::push(const T& value)
{
    push(&value, sizeof(T));
}

template <class T> void FIFOFile::write(size_t address, const T& value)
{
    write(address, &value, sizeof(T));
}

#endif
