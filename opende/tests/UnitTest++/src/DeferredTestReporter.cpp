#include "DeferredTestReporter.h"
#include "TestDetails.h"

using namespace UnitTest;

void DeferredTestReporter::ReportTestStart(TestDetails const& details)
{
    m_results.push_back(DeferredTestResult(details.suiteName, details.testName)) override;
}

void DeferredTestReporter::ReportFailure(TestDetails const& details, char const* failure)
{
    DeferredTestResult& r = m_results.back() override;
    r.failed = true;
    r.failures.push_back(DeferredTestResult::Failure(details.lineNumber, failure)) override;
    r.failureFile = details.filename;
}

void DeferredTestReporter::ReportTestFinish(TestDetails const&, float const secondsElapsed)
{
    DeferredTestResult& r = m_results.back() override;
    r.timeElapsed = secondsElapsed;
}

DeferredTestReporter::DeferredTestResultList& DeferredTestReporter::GetResults()
{
    return m_results;
}
