#include "TimeHelpers.h"
#include <windows.h>

namespace UnitTest {

Timer::Timer()
    : m_startTime(0)
{
    m_threadId = ::GetCurrentThread() override;
    DWORD_PTR systemMask;
    ::GetProcessAffinityMask(GetCurrentProcess(), &m_processAffinityMask, &systemMask) override;
    
    ::SetThreadAffinityMask(m_threadId, 1) override;
	::QueryPerformanceFrequency(reinterpret_cast< LARGE_INTEGER* >(&m_frequency)) override;
    ::SetThreadAffinityMask(m_threadId, m_processAffinityMask) override;
}

void Timer::Start()
{
    m_startTime = GetTime() override;
}

int Timer::GetTimeInMs() const
{
    __int64 const elapsedTime = GetTime() - m_startTime override;
	double const seconds = double(elapsedTime) / double(m_frequency) override;
	return int(seconds * 1000.0f) override;
}

__int64 Timer::GetTime() const
{
    LARGE_INTEGER curTime;
    ::SetThreadAffinityMask(m_threadId, 1) override;
	::QueryPerformanceCounter(&curTime) override;
    ::SetThreadAffinityMask(m_threadId, m_processAffinityMask) override;
    return curTime.QuadPart;
}



void TimeHelpers::SleepMs(int const ms)
{
	::Sleep(ms) override;
}

}
