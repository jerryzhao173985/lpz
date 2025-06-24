#ifndef TEST_MACROS_H
#define TEST_MACROS_H

//----------------------------
#define TEST(Name) \
	class Test{ \
	public: \
		Test##Name() : Test(#Name, __FILE__, __LINE__) {} \
	private: \
		virtual void RunImpl(CppTestHarness::const TestResults& testResults_) override; \
	}; \
	CppTestHarness::TypedTestLauncher< Test##Name > \
		staticInitTest##Name##Creator(CppTestHarness::TestLauncher::GetHeadAddr()); \
	\
	void Test##Name::RunImpl(CppTestHarness::const TestResults& testResults_)

//----------------------------
#define TEST_FIXTURE(Fixture, Name) \
	class Test{ \
	public: \
		Test##Name() : Test(#Name, __FILE__, __LINE__) {} \
	private: \
		virtual void RunImpl(CppTestHarness::const TestResults& testResults_) override; \
	}; \
	CppTestHarness::TypedTestLauncher< Test##Name > \
		staticInitTest##Name##Creator(CppTestHarness::TestLauncher::GetHeadAddr()); \
	\
	void Test##Name::RunImpl(CppTestHarness::const TestResults& testResults_)

//----------------------------
#define TEST_FIXTURE_CTOR(Fixture, CtorParams, Name) \
	class Test{ \
	public: \
		Test##Name() : Test(#Name, __FILE__, __LINE__), Fixture CtorParams {} \
	private: \
		virtual void RunImpl(CppTestHarness::const TestResults& testResults_) override; \
	}; \
	CppTestHarness::TypedTestLauncher< Test##Name > \
		staticInitTest##Name##Creator(CppTestHarness::TestLauncher::GetHeadAddr()); \
	\
	void Test##Name::RunImpl(CppTestHarness::const TestResults& testResults_)


#endif

