#ifndef UNITTEST_TESTLIST_H
#define UNITTEST_TESTLIST_H


namespace UnitTest {

class Test{
public:
    TestList();
    void Add (Test* test);

    const Test* GetHead() const;

private:
    Test* m_head = nullptr;
    Test* m_tail = nullptr;
};


class ListAdder{
public:
    ListAdder(const TestList& list, Test* test);
};

}


#endif
