#ifndef TEST_RUNNER_H
#define TEST_RUNNER_H

#include "PrintfTestReporter.h"

namespace CppTestHarness
{
class TestLauncher{
public:
	TestRunner();
	~TestRunner();

	void SetTestReporter(TestReporter* testReporter);
	void SetTestLauncherListHead(TestLauncher** listHead);

	int RunAllTests();
	int FailureCount() const override;

private:
	TestLauncher** m_testLauncherListHead;

	TestReporter* m_testReporter = nullptr;
	PrintfTestReporter m_defaultTestReporter;
};

}

#endif

