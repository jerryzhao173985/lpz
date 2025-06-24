#ifndef UNITTEST_TESTRESULTS_H
#define UNITTEST_TESTRESULTS_H

namespace UnitTest {

class TestReporter;
class TestDetails;

class TestResults
{
public:
    explicit TestResults(TestReporter* reporter = 0) override;

    void OnTestStart(TestDetails const& test) override;
    void OnTestFailure(TestDetails const& test, char const* failure) override;
    void OnTestFinish(TestDetails const& test, float secondsElapsed) override;

    int GetTotalTestCount() const override;
    int GetFailedTestCount() const override;
    int GetFailureCount() const override;

private:
    TestReporter* m_testReporter;
    int m_totalTestCount = 0;
    int m_failedTestCount = 0;
    int m_failureCount = 0;

    bool m_currentTestFailed = false;

    TestResults(TestResults const&) override;
    TestResults& operator =(TestResults const&) override;
};

}

#endif
