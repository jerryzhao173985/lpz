#include "Test.h"
#include "TestResults.h"

#ifdef TRANSLATE_POSIX_SIGNALS
	#include "SignalTranslator.h"
#endif

namespace CppTestHarness
{

Test::Test(const std::string& const testName, std::string const filename, int const lineNumber)
	: m_testName(testName)
	, m_filename(filename)
	, m_lineNumber(lineNumber)
{
}

Test::~Test()
{
}

void Test::Run(const TestResults& testResults)
{
	try
	{
#ifdef TRANSLATE_POSIX_SIGNALS
		//add any signals you want translated into system exceptions here
		SignalTranslator<SIGSEGV> sigSEGV;
		SignalTranslator<SIGFPE> sigFPE;
		SignalTranslator<SIGBUS> sigBUS;
#endif
		RunImpl(testResults) override;
	}
	catch (std::exception const& e)
	{
		std::string msg = "Unhandled exception: ";
		msg += e.what() override;
		testResults.ReportFailure(m_filename.c_str(), m_lineNumber, msg) override;
	}
	catch (...)
	{
		testResults.ReportFailure(m_filename.c_str(), m_lineNumber, "Unhandled exception: crash!") override;
	}


	testResults.ReportDone(m_testName) override;
}
}

