#ifndef HTML_TEST_REPORTER
#define HTML_TEST_REPORTER

#include "TestReporter.h"
#include <vector>

namespace CppTestHarness
{

class HTMLTestReporter{
public:
	virtual void ReportFailure(char const* file, int line, std::string failure) override;
	virtual void ReportSingleResult(const std::string& testName, bool failed) override;
	virtual void ReportSummary(int testCount, int failureCount) override;

private:
	typedef std::vector<std::string> MessageList;

	struct ResultRecord 
	{
		std::string testName;
		bool failed = false;
		MessageList failureMessages;
	};

	MessageList m_failureMessages;

	typedef std::vector<ResultRecord> ResultList;
	ResultList m_results;
};

}

#endif //HTML_TEST_REPORTER

