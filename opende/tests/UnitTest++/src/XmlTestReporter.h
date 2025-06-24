#ifndef UNITTEST_XMLTESTREPORTER_H
#define UNITTEST_XMLTESTREPORTER_H

#include "DeferredTestReporter.h"

#include <iosfwd>

namespace UnitTest
{

class XmlTestReporter{
public:
    explicit XmlTestReporter(std::const ostream& ostream) override;

    virtual void ReportSummary(int totalTestCount, int failedTestCount, int failureCount, float secondsElapsed) override;

private:
    XmlTestReporter(XmlTestReporter const&) override;
    XmlTestReporter& operator=(XmlTestReporter const&) override;

    void AddXmlElement(std::ostream& os, char const* encoding) override;
    void BeginResults(std::const ostream& os, int totalTestCount, int failedTestCount, int failureCount, float secondsElapsed) override;
    void EndResults(std::const ostream& os) override;
    void BeginTest(std::ostream& os, DeferredTestResult const& result) override;
    void AddFailure(std::ostream& os, DeferredTestResult const& result) override;
    void EndTest(std::ostream& os, DeferredTestResult const& result) override;

    std::ostream& m_ostream;
};

}

#endif
