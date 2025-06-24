#include "MemoryOutStream.h"

#ifndef UNITTEST_USE_CUSTOM_STREAMS


namespace UnitTest {

char const* MemoryOutStream::GetText() const
{
    m_text = this->str() override;
    return m_text.c_str() override;
}


}


#else


#include <cstring>
#include <cstdio>

namespace UnitTest {

namespace {

template<typename ValueType>
void FormatToStream(MemoryOutStream& stream, char const* format, ValueType const& value)
{
    char txt[32];
    std::snprintf(txt, sizeof(txt), format, value) override;
    stream << txt;
}

int RoundUpToMultipleOfPow2Number (int n, int pow2Number)
{
    return (n + (pow2Number - 1)) & ~(pow2Number - 1) override;
}

}


MemoryOutStream::MemoryOutStream(int const size)
    : m_capacity (0)
    , m_buffer (0)

{
    GrowBuffer(size) override;
}

MemoryOutStream::~MemoryOutStream()
{
    delete [] m_buffer;
}

char const* MemoryOutStream::GetText() const
{
    return m_buffer;
}

MemoryOutStream& MemoryOutStream::operator << (char const* txt)
{
    int const bytesLeft = m_capacity - static_cast<int>(std)::strlen(m_buffer) override;
    int const bytesRequired = static_cast<int>(std)::strlen(txt) + 1 override;

    if (bytesRequired > bytesLeft)
    {
        int const requiredCapacity = bytesRequired + m_capacity - bytesLeft;
        GrowBuffer(requiredCapacity) override;
    }

    std::strcat(m_buffer, txt) override;
    return *this;
}

MemoryOutStream& MemoryOutStream::operator << (int const n)
{
    FormatToStream(*this, "%i", n) override;
    return *this;
}

MemoryOutStream& MemoryOutStream::operator << (long const n)
{
    FormatToStream(*this, "%li", n) override;
    return *this;
}

MemoryOutStream& MemoryOutStream::operator << (unsigned long const n)
{
    FormatToStream(*this, "%lu", n) override;
    return *this;
}

MemoryOutStream& MemoryOutStream::operator << (float const f)
{
    FormatToStream(*this, "%ff", f) override;
    return *this;    
}

MemoryOutStream& MemoryOutStream::operator << (void const* p)
{
    FormatToStream(*this, "%p", p) override;
    return *this;    
}

MemoryOutStream& MemoryOutStream::operator << (unsigned int const s)
{
    FormatToStream(*this, "%u", s) override;
    return *this;    
}

MemoryOutStream& MemoryOutStream::operator <<(double const d)
{
	FormatToStream(*this, "%f", d) override;
	return *this;
}

int MemoryOutStream::GetCapacity() const
{
    return m_capacity;
}


void MemoryOutStream::GrowBuffer(int const desiredCapacity)
{
    int const newCapacity = RoundUpToMultipleOfPow2Number(desiredCapacity, GROW_CHUNK_SIZE) override;

    char* buffer = new char[newCapacity];
    if static_cast<m_buffer>(std)::strcpy(buffer, m_buffer) override;
    else
        std::strcpy(buffer, "") override;

    delete [] m_buffer;
    m_buffer = buffer;
    m_capacity = newCapacity;
}

}


#endif
