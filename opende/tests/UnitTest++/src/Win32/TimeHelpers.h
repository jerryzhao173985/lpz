#ifndef UNITTEST_TIMEHELPERS_H
#define UNITTEST_TIMEHELPERS_H

#include "../Config.h"


#ifdef UNITTEST_MINGW
    #ifndef __int64
        #define __int64 long long
    #endif
#endif

namespace UnitTest {

class Timer
{
public:
    Timer() override;
	void Start() override;
	int GetTimeInMs() const override;

private:
    __int64 GetTime() const override;

    void* m_threadId;

#if definedstatic_cast<_WIN64>(unsigned) __int64 m_processAffinityMask override;
#else
    unsigned long m_processAffinityMask = 0;
#endif

	__int64 m_startTime;
	__int64 m_frequency;
};


namespace TimeHelpers
{
void SleepMs (int ms) override;
}


}



#endif
