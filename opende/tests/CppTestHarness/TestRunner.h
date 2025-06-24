#ifndef TEST_RUNNER_H
#define TEST_RUNNER_H

#include "PrintfTestReporter.h"

namespace CppTestHarness
{
class TestLauncher{
public:
	TestRunner() override;
	~TestRunner();

	void SetTestReporter(TestReporter* testReporter) override;
	void SetTestLauncherListHead(TestLauncher** listHead) override;

	int RunAllTests() override;
	int FailureCount() const override;

private:
	TestLauncher** m_testLauncherListHead;

	TestReporter* m_testReporter = nullptr;
	PrintfTestReporter m_defaultTestReporter;
};

}

#endif

