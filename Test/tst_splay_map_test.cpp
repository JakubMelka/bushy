#include <QString>
#include <QtTest>
#include <QCoreApplication>

#include <map>
#include <type_traits>

#include "MapTestAlgorithms.h"

#include "../Bushy/include/splay_map.h"

class splay_map_test : public QObject
{
    Q_OBJECT

public:
    splay_map_test();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void testConstructors();
    void testIterators();
};

splay_map_test::splay_map_test()
{

}

void splay_map_test::initTestCase()
{
    std::map<int, char> m;
    m.get_allocator();
}

void splay_map_test::cleanupTestCase()
{

}

void splay_map_test::testConstructors()
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

    {
        std::allocator<std::pair<int, char>> allocator;

        // Constructor with comparator
        TestMap testMap(allocator);
        StandardMap standardMap(allocator);

        test_map_equality<TestMap, StandardMap>(testMap, standardMap);
    }

    {
        std::less<int> comparator;
        std::allocator<std::pair<int, char>> allocator;

        // Range creation constructor
        StandardMap standardMap = { {1, 'a'}, {2, 'b'}, {3, 'c'} };
        TestMap testMap(standardMap.cbegin(), standardMap.cend(), comparator, allocator);


        test_map_equality<TestMap, StandardMap>(testMap, standardMap);
    }

    {
        std::allocator<std::pair<int, char>> allocator;

        // Range creation constructor
        StandardMap standardMap = { {1, 'a'}, {2, 'b'}, {3, 'c'} };
        TestMap testMap(standardMap.cbegin(), standardMap.cend(), allocator);


        test_map_equality<TestMap, StandardMap>(testMap, standardMap);
    }

    {
        // Copy constructor
        StandardMap standardMap = { {1, 'a'}, {2, 'b'}, {3, 'c'} };
        TestMap other(standardMap.cbegin(), standardMap.cend());
        TestMap testMap(other);

        test_map_equality<TestMap, StandardMap>(testMap, standardMap);
    }

    {
        std::allocator<std::pair<int, char>> allocator;

        // Copy constructor
        StandardMap standardMap = { {1, 'a'}, {2, 'b'}, {3, 'c'} };
        TestMap other(standardMap.cbegin(), standardMap.cend(), allocator);
        TestMap testMap(other, allocator);

        test_map_equality<TestMap, StandardMap>(testMap, standardMap);
    }

    {
        // Initializer list constructor
        StandardMap standardMap({ {1, 'a'}, {2, 'b'}, {3, 'c'} });
        TestMap testMap({ {1, 'a'}, {2, 'b'}, {3, 'c'} });

        test_map_equality<TestMap, StandardMap>(testMap, standardMap);
    }

    {
        std::allocator<std::pair<int, char>> allocator;

        // Initializer list constructor
        StandardMap standardMap({ {1, 'a'}, {2, 'b'}, {3, 'c'} });
        TestMap testMap({ {1, 'a'}, {2, 'b'}, {3, 'c'} }, allocator);

        test_map_equality<TestMap, StandardMap>(testMap, standardMap);
    }
}

void splay_map_test::testIterators()
{
    {
        // Default constructed iterator must be equal to the end iterator.
        bushy::splay_map<int, char>::const_iterator itEmpty;

        bushy::splay_map<int, char> map;
        QVERIFY(map.cend() == itEmpty);
        QVERIFY(map.end() == itEmpty);
    }

    {
        // Default constructed iterator must be equal to the end iterator.
        bushy::splay_map<int, char>::iterator itEmpty;

        bushy::splay_map<int, char> map;
        QVERIFY(map.cend() == itEmpty);
        QVERIFY(map.end() == itEmpty);
    }

    {
        // Empty map begin iterator must be equal to the end iterator.
        bushy::splay_map<int, char> map;

        QVERIFY(map.begin() == map.end());
        QVERIFY(map.cbegin() == map.cend());
        QVERIFY(map.rbegin() == map.rend());
        QVERIFY(map.crbegin() == map.crend());
    }

    {
        // Iterator conversion test
        bushy::splay_map<int, char> map = { {1, 'a'}, {2, 'b'}, {3, 'c'} };

        bushy::splay_map<int, char>::iterator it = map.begin();
        bushy::splay_map<int, char>::const_iterator itConst = it;
        QVERIFY(it == itConst);
    }

    {
        // Iterator conversion test
        bushy::splay_map<int, char> map = { {1, 'a'}, {2, 'b'}, {3, 'c'} };
        std::map<int, char> standard_map = { {1, 'a'}, {2, 'b'}, {3, 'c'} };

        bushy::splay_map<int, char>::iterator it = map.begin();
        std::map<int, char>::iterator itStandard = standard_map.begin();

        it->second = 'q';
        itStandard->second = 'q';

        test_map_equality<bushy::splay_map<int, char>, std::map<int, char>>(map, standard_map);
    }

    {
        // Iterator decrement/increment test
        bushy::splay_map<int, char> map = { {1, 'a'}, {2, 'b'}, {3, 'c'} };

        bushy::splay_map<int, char>::const_iterator it = map.find(2);
        QVERIFY(it != map.cend());
        QVERIFY(it != map.end());
        QVERIFY(it->first == 2);
        QVERIFY(it->second == 'b');

        bushy::splay_map<int, char>::const_iterator it2 = it;
        QVERIFY(--(++it2) == it);

        QVERIFY(--it2 != it);
        QVERIFY(++(++it2) != it);
        QVERIFY(it != map.cend());
    }

    {
        // Static assertions on types
        {
            bushy::splay_map<int, char>::const_iterator it;
            static_assert(std::is_same<bushy::splay_map<int, char>::const_iterator&, decltype(--it)>::value, "BidirectionalIterator requirements not met!");
            static_assert(std::template is_convertible<decltype(it--), const bushy::splay_map<int, char>::const_iterator&>::value, "BidirectionalIterator requirements not met!");
            static_assert(std::is_same<decltype(*it--), bushy::splay_map<int, char>::const_reference>::value, "BidirectionalIterator requirements not met!");
            static_assert(std::is_same<bushy::splay_map<int, char>::const_iterator&, decltype(++it)>::value, "BidirectionalIterator requirements not met!");
            static_assert(std::template is_convertible<decltype(it++), const bushy::splay_map<int, char>::const_iterator&>::value, "BidirectionalIterator requirements not met!");
            static_assert(std::is_same<decltype(*it++), bushy::splay_map<int, char>::const_reference>::value, "BidirectionalIterator requirements not met!");
            static_assert(std::is_same<std::template iterator_traits<bushy::splay_map<int, char>::const_iterator>::reference, bushy::splay_map<int, char>::const_reference>::value, "ForwardIterator requirements not met!");

            // DefaultConstructible requierements
            static_assert(std::template is_default_constructible<bushy::splay_map<int, char>::const_iterator>::value, "DefaultConstructible requirements not met!");

            bushy::splay_map<int, char>::const_iterator it2;
            bushy::splay_map<int, char>::const_iterator it3{};
            QVERIFY(it == it2);
            QVERIFY(it == it3);
            QVERIFY((it == bushy::splay_map<int, char>::const_iterator()));
            QVERIFY((it == bushy::splay_map<int, char>::const_iterator{}));
        }

        {
            bushy::splay_map<int, char>::iterator it;
            static_assert(std::is_same<bushy::splay_map<int, char>::iterator&, decltype(--it)>::value, "BidirectionalIterator requirements not met!");
            static_assert(std::template is_convertible<decltype(it--), const bushy::splay_map<int, char>::iterator&>::value, "BidirectionalIterator requirements not met!");
            static_assert(std::is_same<decltype(*it--), bushy::splay_map<int, char>::reference>::value, "BidirectionalIterator requirements not met!");
            static_assert(std::is_same<bushy::splay_map<int, char>::iterator&, decltype(++it)>::value, "BidirectionalIterator requirements not met!");
            static_assert(std::template is_convertible<decltype(it++), const bushy::splay_map<int, char>::iterator&>::value, "BidirectionalIterator requirements not met!");
            static_assert(std::is_same<decltype(*it++), bushy::splay_map<int, char>::reference>::value, "BidirectionalIterator requirements not met!");
            static_assert(std::is_same<std::template iterator_traits<bushy::splay_map<int, char>::iterator>::reference, bushy::splay_map<int, char>::reference>::value, "ForwardIterator requirements not met!");

            // DefaultConstructible requierements
            static_assert(std::template is_default_constructible<bushy::splay_map<int, char>::iterator>::value, "DefaultConstructible requirements not met!");

            bushy::splay_map<int, char>::iterator it2;
            bushy::splay_map<int, char>::iterator it3{};
            QVERIFY(it == it2);
            QVERIFY(it == it3);
            QVERIFY((it == bushy::splay_map<int, char>::iterator()));
            QVERIFY((it == bushy::splay_map<int, char>::iterator{}));
        }
    }
}

QTEST_MAIN(splay_map_test)

#include "tst_splay_map_test.moc"
