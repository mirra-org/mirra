#include "FS.h"

namespace mirra::fs
{
    class FIFOFile : mirra::fs::File
    {
    protected:
        size_t head;
        size_t tail;

    private:
        //@brief Updates the tail and head.
        virtual size_t segmentor(size_t size);

    public:
        FIFOFile(const char* name);
        virtual ~FIFOFile();

        size_t size() const;
        bool inFile(size_t address) const;

        void read(size_t address, void* buffer, size_t size) const;
        template <class T> T read(size_t address) const;

        void push(const void* buffer, size_t size);
        template <class T> void push(const T& value);

        void write(size_t address, const void* buffer, size_t size);
        template <class T> void write(size_t address, const T& value);

        using File::flush;
    };
#include "FIFOFile.tpp"
};