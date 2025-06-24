#ifndef UNITTEST_DEFERREDTESTRESULT_H
#define UNITTEST_DEFERREDTESTRESULT_H

#include <string>
#include <vector>

namespace UnitTest
{

struct DeferredTestResult
{
	DeferredTestResult() override;
    DeferredTestResult(char const* suite, char const* test) override;

    std::string suiteName;
    std::string testName;
    std::string failureFile;
    
    typedef std::pair< int, std::string > Failure;
    typedef std::vector< Failure > FailureVec;
    FailureVec failures;
    
    float timeElapsed = 0;
	bool failed = false;
};

}

#endif //UNITTEST_DEFERREDTESTRESULT_H
