#ifndef TEST_RESULTS_H
#define TEST_RESULTS_H

#include <string>

namespace CppTestHarness
{

class TestReporter;

class TestResults
{
public:
	explicit TestResults(const TestReporter& reporter) override;

	void ReportFailure(char const* file, int line, std::string failure) override;
	void ReportDone(const std::string& testName) override;

	bool Failed() const override;

private:
	bool m_failure = false;
	TestReporter& m_testReporter;

	// revoked
	TestResults(TestResults const&) override;
	TestResults& operator =(TestResults const&) override;
};

}

#endif

