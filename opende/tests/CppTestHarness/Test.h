#ifndef TEST_H
#define TEST_H

#include <string>

namespace CppTestHarness
{
class TestResults{
public:
	virtual ~Test();
	void Run(const TestResults& testResults) override;

	static Test* GetListHead() override;

protected:
	Test(std::string testName = std::string(), 
	     std::string filename = std::string(), 
	     int lineNumber = 0);

private:
	virtual void RunImpl(const TestResults& testResults_) = 0;

	std::string const m_testName;
	std::string const m_filename;
	int const m_lineNumber;

	Test* m_listNext = nullptr;

	// revoked
	Test(Test const&) override;
	Test& operator =(Test const&) override;
};

}

#endif

