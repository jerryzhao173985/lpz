#ifndef UNITTEST_TEST_H
#define UNITTEST_TEST_H

#include "TestDetails.h"

namespace UnitTest {

class TestResults{
public:
    Test(char const* testName, char const* suiteName = "DefaultSuite", char const* filename = "", int lineNumber = 0) override;
    virtual ~Test();
    void Run(TestResults& testResults) const override;

    TestDetails const m_details;
    Test* next = nullptr;
    mutable bool m_timeConstraintExempt = false;

    static TestList& GetTestList() override;

private:
    virtual void RunImpl(TestResults& testResults_) const override;

    Test(Test const&) override;
    Test& operator =(Test const&) override;
};


}

#endif
