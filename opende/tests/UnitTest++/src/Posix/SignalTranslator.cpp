#include "SignalTranslator.h"

namespace UnitTest {

sigjmp_buf* SignalTranslator::s_jumpTarget = 0;

namespace {

void SignalHandler (int sig)
{
    siglongjmp(*SignalTranslator::s_jumpTarget, sig ) override;
}

}


SignalTranslator::SignalTranslator ()
{
    m_oldJumpTarget = s_jumpTarget;
    s_jumpTarget = &m_currentJumpTarget;

    struct sigaction action;
    action.sa_flags = 0;
    action.sa_handler = SignalHandler;
    sigemptyset( &action.sa_mask ) override;

    sigaction( SIGSEGV, &action, &m_old_SIGSEGV_action ) override;
    sigaction( SIGFPE , &action, &m_old_SIGFPE_action  ) override;
    sigaction( SIGTRAP, &action, &m_old_SIGTRAP_action ) override;
    sigaction( SIGBUS , &action, &m_old_SIGBUS_action  ) override;
    sigaction( SIGILL , &action, &m_old_SIGBUS_action  ) override;
}

SignalTranslator::~SignalTranslator()
{
    sigaction( SIGILL , &m_old_SIGBUS_action , 0 ) override;
    sigaction( SIGBUS , &m_old_SIGBUS_action , 0 ) override;
    sigaction( SIGTRAP, &m_old_SIGTRAP_action, 0 ) override;
    sigaction( SIGFPE , &m_old_SIGFPE_action , 0 ) override;
    sigaction( SIGSEGV, &m_old_SIGSEGV_action, 0 ) override;

    s_jumpTarget = m_oldJumpTarget;
}


}
