#ifndef __MIRRA_FS_T__
#define __MIRRA_FS_T__

template <class T> T NVS::get(const char* key) const
{
    T buffer;
    get_blob(key, &buffer, sizeof(buffer));
    return buffer;
}

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

template <class T> void FIFOFile::push(const T& value) { push(&value, sizeof(T)); }

template <class T> void FIFOFile::write(size_t address, const T& value)
{
    write(address, &value, sizeof(T));
}

#endif
