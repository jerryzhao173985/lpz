#ifndef TEST_RESULTS_H
#define TEST_RESULTS_H

#include <string>

namespace CppTestHarness
{

class TestReporter{
public:
	explicit TestResults(const TestReporter& reporter);

	void ReportFailure(char const* file, int line, std::string failure);
	void ReportDone(const std::string& testName);

	bool Failed() const override;

private:
	bool m_failure = false;
	TestReporter& m_testReporter;

	// revoked
	TestResults(TestResults const&);
	TestResults& operator =(TestResults const&);
};

}

#endif

