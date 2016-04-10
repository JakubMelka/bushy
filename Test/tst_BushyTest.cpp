#include <QString>
#include <QtTest>
#include <QCoreApplication>

#include <map>

#include "MapTestAlgorithms.h"

#include "../Bushy/include/splay_map.h"

class BushyTest : public QObject
{
    Q_OBJECT

public:
    BushyTest();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void testConstructors();
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

void BushyTest::testConstructors()
{
    using Compare = std::less<int>;
    using Allocator = std::allocator<std::pair<const int, char>>;
    using TestMap = bushy::splay_map<int, char>;
    using StandardMap = std::map<int, char>;

    {
        // Explicit empty constructor
        bushy::splay_map<int, char> testMap;
        std::map<int, char> standardMap;

        test_map_equality<TestMap, StandardMap>(testMap, standardMap);
    }

    {
        std::less<int> comparator;

        // Constructor with comparator
        TestMap testMap(comparator);
        StandardMap standardMap(comparator);

        test_map_equality<TestMap, StandardMap>(testMap, standardMap);
    }
}

QTEST_MAIN(BushyTest)

#include "tst_BushyTest.moc"
