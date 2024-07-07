template <> uint8_t NVS::get(const char* key) const { return get_u8(key); }
template <> uint16_t NVS::get(const char* key) const { return get_u16(key); }
template <> uint32_t NVS::get(const char* key) const { return get_u32(key); }
template <> uint64_t NVS::get(const char* key) const { return get_u64(key); }

template <> int8_t NVS::get(const char* key) const { return get_i8(key); }
template <> int16_t NVS::get(const char* key) const { return get_i16(key); }
template <> int32_t NVS::get(const char* key) const { return get_i32(key); }
template <> int64_t NVS::get(const char* key) const { return get_i64(key); }

template <class T> T NVS::get(const char* key) const
{
    T buffer;
    get_blob(key, &buffer, sizeof(buffer));
    return buffer;
}

template <> void NVS::set(const char* key, const uint8_t& value) { set_u8(key, value); }
template <> void NVS::set(const char* key, const uint16_t& value) { set_u16(key, value); }
template <> void NVS::set(const char* key, const uint32_t& value) { set_u32(key, value); }
template <> void NVS::set(const char* key, const uint64_t& value) { set_u64(key, value); }

template <> void NVS::set(const char* key, const int8_t& value) { set_i8(key, value); }
template <> void NVS::set(const char* key, const int16_t& value) { set_i16(key, value); }
template <> void NVS::set(const char* key, const int32_t& value) { set_i32(key, value); }
template <> void NVS::set(const char* key, const int64_t& value) { set_i64(key, value); }

template <> void NVS::set(const char* key, const char* const& value) { set_str(key, value); }

template <class T> void NVS::set(const char* key, T&& value)
{
    set_blob(key, &std::forward<T>(value), sizeof(T));
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

template <class T> void FIFOFile::write(size_t address, const T& value)
{
    write(address, &value, sizeof(T));
}
