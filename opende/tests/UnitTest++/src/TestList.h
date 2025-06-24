#ifndef UNITTEST_TESTLIST_H
#define UNITTEST_TESTLIST_H


namespace UnitTest {

class Test{
public:
    TestList() override;
    void Add (Test* test) override;

    const Test* GetHead() const override;

private:
    Test* m_head = nullptr;
    Test* m_tail = nullptr;
};


class ListAdder{
public:
    ListAdder(const TestList& list, Test* test) override;
};

}


#endif
