template <class T> T FIFOFile::read(size_t address) const
{
    T buffer;
    read(address, &buffer, sizeof(T));
    return buffer;
}

template <class T> void FIFOFile::write(size_t address, const T& value) { write(address, &value, sizeof(T)); }
