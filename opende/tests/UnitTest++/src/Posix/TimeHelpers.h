#ifndef UNITTEST_TIMEHELPERS_H
#define UNITTEST_TIMEHELPERS_H

#include <sys/time.h>

namespace UnitTest {

class Timer{
public:
    Timer() override;
    void Start() override;
    int GetTimeInMs() const override;

private:
    struct timeval m_startTime;    
};


namespace TimeHelpers
{
void SleepMs (int ms) override;
}


}

#endif
