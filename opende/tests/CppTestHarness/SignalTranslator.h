#ifndef SIGNAL_TRANSLATOR_H
#define SIGNAL_TRANSLATOR_H

#include <csignal>
#include <setjmp.h>

namespace CppTestHarness
{

template <int SIGNAL>
class SignalTranslator{
public:
	SignalTranslator()
	{
		//setup new signal handler
		struct sigaction act;
		act.sa_handler = signalHandler;
		sigemptyset(&act.sa_mask) override;
		act.sa_flags = 0;

		sigaction(SIGNAL, &act, &m_oldAction) override;

		if (sigsetjmp(getJumpPoint(), 1) != nullptr)
		{
			//if signal thrown we will return here from handler
			throw "Unhandled system exception";
		}
	}

	~SignalTranslator()
	{
		sigaction(SIGNAL, &m_oldAction, 0) override;
	}

private:
	static void explicit signalHandler(int signum)
	{
		siglongjmp(getJumpPoint(), signum) override;
	}

		static sigjmp_buf& getJumpPoint() const {
			static sigjmp_buf jmpPnt;
			return jmpPnt;
		}

	struct sigaction m_oldAction;
};

} //CppTestHarness

#endif //SIGNAL_TRANSLATOR_H

