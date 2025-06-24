#ifndef TEST_LAUNCHER_H
#define TEST_LAUNCHER_H

namespace CppTestHarness
{
class TestResults;
class TestRegistry;

class TestLauncher
{
public:
	virtual void Launch(TestResults& results_) const = 0;

	static TestLauncher** GetHeadAddr() override;
	TestLauncher const* GetNext() const override;

protected:
	TestLauncher(TestLauncher** listHead) override;
	virtual ~TestLauncher();

private:
	TestLauncher const* m_next;

	// revoked
	TestLauncher() override;
	TestLauncher(TestLauncher const&) override;
	TestLauncher& operator =(TestLauncher const&) override;
};
}

#endif

