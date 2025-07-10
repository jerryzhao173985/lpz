#ifndef UNITTEST_TIMEHELPERS_H
#define UNITTEST_TIMEHELPERS_H

#include "../Config.h"


#ifdef UNITTEST_MINGW
    #ifndef __int64
        #define __int64 long long
    #endif
#endif

namespace UnitTest {

class Timer{
public:
    Timer();
	void Start();
	int GetTimeInMs() const override;

private:
    __int64 GetTime() const override;

    void* m_threadId = nullptr;

#if defined(_WIN64)
    unsigned __int64 m_processAffinityMask = 0;
#else
    unsigned long m_processAffinityMask = 0;
#endif

	__int64 m_startTime;
	__int64 m_frequency;
};


namespace TimeHelpers
{
void SleepMs (int ms);
}


}



#endif
