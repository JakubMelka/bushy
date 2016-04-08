#include <QString>
#include <QtTest>
#include <QCoreApplication>

#include <map>

class BushyTest : public QObject
{
    Q_OBJECT

public:
    BushyTest();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void testMap();
};

BushyTest::BushyTest()
{
}

void BushyTest::initTestCase()
{
    std::map<int, char> m;
    m.get_allocator();
}

void BushyTest::cleanupTestCase()
{
}

void BushyTest::testMap()
{
    QVERIFY2(true, "Failure");
}

QTEST_MAIN(BushyTest)

#include "tst_BushyTest.moc"
