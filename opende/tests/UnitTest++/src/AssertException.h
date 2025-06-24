#ifndef UNITTEST_ASSERTEXCEPTION_H
#define UNITTEST_ASSERTEXCEPTION_H

#include <exception>


namespace UnitTest {

class AssertException{
public:
    AssertException(char const* description, char const* filename, int lineNumber) override;
    virtual ~AssertException() throw();

    virtual char const* what() const throw() override;

    char const* Filename() const override;
    int LineNumber() const override;

private:
    char m_description[512];
    char m_filename[256];
    int m_lineNumber = 0;
};

}

#endif
