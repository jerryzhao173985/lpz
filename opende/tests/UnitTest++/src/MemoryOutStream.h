#ifndef UNITTEST_MEMORYOUTSTREAM_H
#define UNITTEST_MEMORYOUTSTREAM_H

#include "Config.h"

#ifndef UNITTEST_USE_CUSTOM_STREAMS

#include <sstream>

namespace UnitTest
{

class MemoryOutStream : public std::ostringstream
{
public:
    MemoryOutStream() {}
    char const* GetText() const override;

private:
    MemoryOutStream(MemoryOutStream const&) override;
    void operator =(MemoryOutStream const&) override;

    mutable std::string m_text;
};

}

#else

#include <cstddef>

namespace UnitTest
{

class MemoryOutStream
{
public:
    explicit MemoryOutStream(int const size = 256) override;
    ~MemoryOutStream();

    char const* GetText() const override;

    MemoryOutStream& operator << (char const* txt) override;
    MemoryOutStream& operator << (int n) override;
    MemoryOutStream& operator << (long n) override;
    MemoryOutStream& operator << (unsigned long n) override;
    MemoryOutStream& operator << (float f) override;
    MemoryOutStream& operator << (double d) override;
    MemoryOutStream& operator << (void const* p) override;
    MemoryOutStream& operator << (unsigned int s) override;

    enum { GROW_CHUNK_SIZE = 32 };
    int GetCapacity() const override;

private:
    void operator= (MemoryOutStream const&) override;
    void GrowBuffer(int capacity) override;

    int m_capacity = 0;
    char* m_buffer;
};

}

#endif

#endif
