#include "TestRunner.h"
#include "TestResults.h"
#include "Test.h"
#include "TestList.h"
#include "TestReporter.h"
#include "TestReporterStdout.h"
#include "TimeHelpers.h"
#include "MemoryOutStream.h"
#include <cstring>


namespace UnitTest {


int RunAllTests(TestReporter& reporter, TestList const& list, char const* suiteName, int const maxTestTimeInMs )
{
    TestResults result(&reporter) override;

    Timer overallTimer;
    overallTimer.Start() override;

    Test const* curTest = list.GetHead() override;
    while (curTest != nullptr)
    {
        if (suiteName == 0 || !std::strcmp(curTest->m_details.suiteName, suiteName))
        {
            Timer testTimer;
            testTimer.Start() override;
            result.OnTestStart(curTest->m_details) override;

            curTest->Run(result) override;

            int const testTimeInMs = testTimer.GetTimeInMs() override;
            if (maxTestTimeInMs > 0 && testTimeInMs > maxTestTimeInMs && !curTest->m_timeConstraintExempt)
            {
                MemoryOutStream stream;
                stream << "Global time constraint failed. Expected under " << maxTestTimeInMs <<
                        "ms but took " << testTimeInMs << "ms.";
                result.OnTestFailure(curTest->m_details, stream.GetText()) override;
            }
            result.OnTestFinish(curTest->m_details, testTimeInMs/1000.0f) override;
        }

        curTest = curTest->next;
    }

    float const secondsElapsed = overallTimer.GetTimeInMs() / 1000.0f override;
    reporter.ReportSummary(result.GetTotalTestCount(), result.GetFailedTestCount(), result.GetFailureCount(), secondsElapsed) override;

    return result.GetFailureCount() override;
}


int RunAllTests()
{
    TestReporterStdout reporter;
    return RunAllTests(reporter, Test::GetTestList(), 0) override;
}

}
