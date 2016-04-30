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
    void testAssignOperators();
    void testAccessOperators();
    void testInsertAndEmplaceOperations();
    void testMiscellanneousOperations();
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
        // Move constructor
        StandardMap standardMap = { {1, 'a'}, {2, 'b'}, {3, 'c'} };
        TestMap other(standardMap.cbegin(), standardMap.cend());
        TestMap testMap(std::move(other));

        test_map_equality<TestMap, StandardMap>(testMap, standardMap);
    }

    {
        std::allocator<std::pair<int, char>> allocator;

        // Move constructor
        StandardMap standardMap = { {1, 'a'}, {2, 'b'}, {3, 'c'} };
        TestMap other(standardMap.cbegin(), standardMap.cend(), allocator);
        TestMap testMap(std::move(other), allocator);

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

void splay_map_test::testAssignOperators()
{
    using TestMap = bushy::splay_map<int, char>;
    using StandardMap = std::map<int, char>;

    {
        // Assign operator I.
        StandardMap standardMap = { {1, 'a'}, {2, 'b'}, {3, 'c'} };
        TestMap other(standardMap.cbegin(), standardMap.cend());
        TestMap testMap = other;

        test_map_equality<TestMap, StandardMap>(testMap, standardMap);
    }

    {
        // Assign operator II.
        StandardMap standardMap = { {1, 'a'}, {2, 'b'}, {3, 'c'} };
        TestMap other(standardMap.cbegin(), standardMap.cend());
        TestMap testMap = std::move(other);

        test_map_equality<TestMap, StandardMap>(testMap, standardMap);
    }

    {
        // Assign operator III.
        StandardMap standardMap = { {1, 'a'}, {2, 'b'}, {3, 'c'} };
        TestMap other(standardMap.cbegin(), standardMap.cend());
        TestMap testMap;
        testMap = other;

        test_map_equality<TestMap, StandardMap>(testMap, standardMap);
    }

    {
        // Assign operator IV.
        StandardMap standardMap = { {1, 'a'}, {2, 'b'}, {3, 'c'} };
        TestMap other(standardMap.cbegin(), standardMap.cend());
        TestMap testMap;
        testMap = std::move(other);

        test_map_equality<TestMap, StandardMap>(testMap, standardMap);
    }

    {
        // Assign operator V.
        StandardMap standardMap = { {1, 'a'}, {2, 'b'}, {3, 'c'} };
        TestMap testMap;
        testMap = { {1, 'a'}, {2, 'b'}, {3, 'c'} };

        test_map_equality<TestMap, StandardMap>(testMap, standardMap);
    }

    {
        // Assign operator VI.
        StandardMap standardMap = { {1, 'a'}, {2, 'b'}, {3, 'c'} };
        TestMap testMap = { {4, 'a'}, {5, 'b'}, {6, 'c'} };
        testMap = { {1, 'a'}, {2, 'b'}, {3, 'c'} };

        test_map_equality<TestMap, StandardMap>(testMap, standardMap);
    }

    {
        // Assign operator VII.
        StandardMap standardMap = { {1, 'a'}, {2, 'b'}, {3, 'c'} };
        TestMap other(standardMap.cbegin(), standardMap.cend());
        TestMap testMap = { {4, 'a'}, {5, 'b'}, {6, 'c'} };
        testMap = other;

        test_map_equality<TestMap, StandardMap>(testMap, standardMap);
    }

    {
        // Assign operator VIII.
        StandardMap standardMap = { {1, 'a'}, {2, 'b'}, {3, 'c'} };
        TestMap other(standardMap.cbegin(), standardMap.cend());
        TestMap testMap = { {4, 'a'}, {5, 'b'}, {6, 'c'} };
        testMap = std::move(other);

        test_map_equality<TestMap, StandardMap>(testMap, standardMap);
    }
}

void splay_map_test::testAccessOperators()
{
    using TestMap = bushy::splay_map<int, char>;
    using StandardMap = std::map<int, char>;

    {
        StandardMap standard_map = { {1, 'a'}, {2, 'b'}, {3, 'c'} };
        TestMap test_map(standard_map.cbegin(), standard_map.cend());
        test_map_equality<TestMap, StandardMap>(test_map, standard_map);

        const TestMap& const_test_map = test_map;

        QVERIFY(const_test_map.at(1) == standard_map.at(1));
        QVERIFY(const_test_map.at(2) == standard_map.at(2));
        QVERIFY(const_test_map.at(3) == standard_map.at(3));

        test_map.at(1) = 'd';
        test_map.at(2) = 'e';
        test_map.at(3) = 'f';

        standard_map.at(1) = 'd';
        standard_map.at(2) = 'e';
        standard_map.at(3) = 'f';

        test_map_equality<TestMap, StandardMap>(test_map, standard_map);

        bool exception_thrown = false;
        try
        {
            standard_map.at(4);
        }
        catch (std::out_of_range)
        {
            exception_thrown = true;
        }

        QVERIFY(exception_thrown);
    }

    {
        StandardMap standard_map;
        TestMap test_map(standard_map.cbegin(), standard_map.cend());
        test_map_equality<TestMap, StandardMap>(test_map, standard_map);

        standard_map[50] = 'a';
        test_map[50] = 'a';
        test_map_equality<TestMap, StandardMap>(test_map, standard_map);

        standard_map[52] = 'b';
        test_map[52] = 'b';
        test_map_equality<TestMap, StandardMap>(test_map, standard_map);

        standard_map[50] = 'c';
        test_map[50] = 'c';
        test_map_equality<TestMap, StandardMap>(test_map, standard_map);

        standard_map[52] = 'd';
        test_map[52] = 'd';
        test_map_equality<TestMap, StandardMap>(test_map, standard_map);
    }
}

void splay_map_test::testInsertAndEmplaceOperations()
{
    {
        using TestMap = bushy::splay_map<int, char>;
        using StandardMap = std::map<int, char>;
        using value_type = std::pair<const int, char>;

        value_type first(1, 'a');
        value_type second(2, 'b');

        StandardMap standard_map;
        TestMap test_map;

        auto i1l = standard_map.insert(first);
        auto i1r = test_map.insert(first);
        test_operation_result_equal(i1l, i1r);
        test_map_equality<TestMap, StandardMap>(test_map, standard_map);

        auto i2l = standard_map.insert(first);
        auto i2r = test_map.insert(first);
        test_operation_result_equal(i2l, i2r);
        test_map_equality<TestMap, StandardMap>(test_map, standard_map);

        first.second = 'c';
        auto i3l = standard_map.insert(first);
        auto i3r = test_map.insert(first);
        test_operation_result_equal(i3l, i3r);
        test_map_equality<TestMap, StandardMap>(test_map, standard_map);

        auto i4l = standard_map.insert(second);
        auto i4r = test_map.insert(second);
        test_operation_result_equal(i4l, i4r);
        test_map_equality<TestMap, StandardMap>(test_map, standard_map);

        auto i5l = standard_map.insert(second);
        auto i5r = test_map.insert(second);
        test_operation_result_equal(i5l, i5r);
        test_map_equality<TestMap, StandardMap>(test_map, standard_map);

        second.second = 'q';
        auto i6l = standard_map.insert(second);
        auto i6r = test_map.insert(second);
        test_operation_result_equal(i6l, i6r);
        test_map_equality<TestMap, StandardMap>(test_map, standard_map);
    }

    {
        using TestMap = bushy::splay_map<int, char>;
        using StandardMap = std::map<int, char>;
        using value_type = std::pair<const int, char>;

        value_type first(1, 'a');
        value_type second(2, 'b');

        StandardMap standard_map;
        TestMap test_map;

        auto i1l = standard_map.insert(first);
        auto i1r = test_map.insert<const value_type&>(first);
        test_operation_result_equal(i1l, i1r);
        test_map_equality<TestMap, StandardMap>(test_map, standard_map);

        auto i2l = standard_map.insert(first);
        auto i2r = test_map.insert<const value_type&>(first);
        test_operation_result_equal(i2l, i2r);
        test_map_equality<TestMap, StandardMap>(test_map, standard_map);

        first.second = 'c';
        auto i3l = standard_map.insert(first);
        auto i3r = test_map.insert<const value_type&>(first);
        test_operation_result_equal(i3l, i3r);
        test_map_equality<TestMap, StandardMap>(test_map, standard_map);

        auto i4l = standard_map.insert(second);
        auto i4r = test_map.insert<const value_type&>(second);
        test_operation_result_equal(i4l, i4r);
        test_map_equality<TestMap, StandardMap>(test_map, standard_map);

        auto i5l = standard_map.insert(second);
        auto i5r = test_map.insert<const value_type&>(second);
        test_operation_result_equal(i5l, i5r);
        test_map_equality<TestMap, StandardMap>(test_map, standard_map);

        second.second = 'q';
        auto i6l = standard_map.insert(second);
        auto i6r = test_map.insert<const value_type&>(second);
        test_operation_result_equal(i6l, i6r);
        test_map_equality<TestMap, StandardMap>(test_map, standard_map);
    }

    {
        using TestMap = bushy::splay_map<int, char>;
        using StandardMap = std::map<int, char>;
        using value_type = std::pair<const int, char>;

        StandardMap standard_map;
        TestMap test_map;

        {
            value_type first(1, 'a');
            value_type second(first);
            auto i1l = standard_map.insert(std::move(first));
            auto i1r = test_map.insert(std::move(second));
            test_operation_result_equal(i1l, i1r);
            test_map_equality<TestMap, StandardMap>(test_map, standard_map);
        }

        {
            value_type first(1, 'a');
            value_type second(first);
            auto i2l = standard_map.insert(std::move(first));
            auto i2r = test_map.insert(std::move(second));
            test_operation_result_equal(i2l, i2r);
            test_map_equality<TestMap, StandardMap>(test_map, standard_map);
        }

        {
            value_type first(1, 'c');
            value_type second(first);
            auto i3l = standard_map.insert(std::move(first));
            auto i3r = test_map.insert(std::move(second));
            test_operation_result_equal(i3l, i3r);
            test_map_equality<TestMap, StandardMap>(test_map, standard_map);
        }

        {
            value_type first(2, 'b');
            value_type second(first);
            auto i4l = standard_map.insert(std::move(first));
            auto i4r = test_map.insert(std::move(second));
            test_operation_result_equal(i4l, i4r);
            test_map_equality<TestMap, StandardMap>(test_map, standard_map);
        }

        {
            value_type first(2, 'b');
            value_type second(first);
            auto i5l = standard_map.insert(std::move(first));
            auto i5r = test_map.insert(std::move(second));
            test_operation_result_equal(i5l, i5r);
            test_map_equality<TestMap, StandardMap>(test_map, standard_map);
        }

        {
            value_type first(2, 'q');
            value_type second(first);
            auto i6l = standard_map.insert(std::move(first));
            auto i6r = test_map.insert(std::move(second));
            test_operation_result_equal(i6l, i6r);
            test_map_equality<TestMap, StandardMap>(test_map, standard_map);
        }
    }

    {
        using TestMap = bushy::splay_map<int, char>;
        using StandardMap = std::map<int, char>;
        using value_type = std::pair<const int, char>;

        value_type first(1, 'a');
        value_type second(2, 'b');

        StandardMap standard_map;
        TestMap test_map;

        auto i1l = standard_map.insert(standard_map.cend(), first);
        auto i1r = test_map.insert(test_map.cend(), first);
        test_iterator_equal(i1l, i1r);
        test_map_equality<TestMap, StandardMap>(test_map, standard_map);

        auto i2l = standard_map.insert(standard_map.cend(), first);
        auto i2r = test_map.insert(test_map.cend(), first);
        test_iterator_equal(i2l, i2r);
        test_map_equality<TestMap, StandardMap>(test_map, standard_map);

        first.second = 'c';
        auto i3l = standard_map.insert(standard_map.cend(), first);
        auto i3r = test_map.insert(test_map.cend(), first);
        test_iterator_equal(i3l, i3r);
        test_map_equality<TestMap, StandardMap>(test_map, standard_map);

        auto i4l = standard_map.insert(standard_map.cend(), second);
        auto i4r = test_map.insert(test_map.cend(), second);
        test_iterator_equal(i4l, i4r);
        test_map_equality<TestMap, StandardMap>(test_map, standard_map);

        auto i5l = standard_map.insert(standard_map.cend(), second);
        auto i5r = test_map.insert(test_map.cend(), second);
        test_iterator_equal(i5l, i5r);
        test_map_equality<TestMap, StandardMap>(test_map, standard_map);

        second.second = 'q';
        auto i6l = standard_map.insert(standard_map.cend(), second);
        auto i6r = test_map.insert(test_map.cend(), second);
        test_iterator_equal(i6l, i6r);
        test_map_equality<TestMap, StandardMap>(test_map, standard_map);
    }

    {
        using TestMap = bushy::splay_map<int, char>;
        using StandardMap = std::map<int, char>;
        using value_type = std::pair<const int, char>;

        value_type first(1, 'a');
        value_type second(2, 'b');

        StandardMap standard_map;
        TestMap test_map;

        auto i1l = standard_map.insert(standard_map.cend(), first);
        auto i1r = test_map.insert<const value_type&>(test_map.cend(), first);
        test_iterator_equal(i1l, i1r);
        test_map_equality<TestMap, StandardMap>(test_map, standard_map);

        auto i2l = standard_map.insert(standard_map.cend(), first);
        auto i2r = test_map.insert<const value_type&>(test_map.cend(), first);
        test_iterator_equal(i2l, i2r);
        test_map_equality<TestMap, StandardMap>(test_map, standard_map);

        first.second = 'c';
        auto i3l = standard_map.insert(standard_map.cend(), first);
        auto i3r = test_map.insert<const value_type&>(test_map.cend(), first);
        test_iterator_equal(i3l, i3r);
        test_map_equality<TestMap, StandardMap>(test_map, standard_map);

        auto i4l = standard_map.insert(standard_map.cend(), second);
        auto i4r = test_map.insert<const value_type&>(test_map.cend(), second);
        test_iterator_equal(i4l, i4r);
        test_map_equality<TestMap, StandardMap>(test_map, standard_map);

        auto i5l = standard_map.insert(standard_map.cend(), second);
        auto i5r = test_map.insert<const value_type&>(test_map.cend(), second);
        test_iterator_equal(i5l, i5r);
        test_map_equality<TestMap, StandardMap>(test_map, standard_map);

        second.second = 'q';
        auto i6l = standard_map.insert(standard_map.cend(), second);
        auto i6r = test_map.insert<const value_type&>(test_map.cend(), second);
        test_iterator_equal(i6l, i6r);
        test_map_equality<TestMap, StandardMap>(test_map, standard_map);
    }

    {
        using TestMap = bushy::splay_map<int, char>;
        using StandardMap = std::map<int, char>;
        using value_type = std::pair<const int, char>;

        StandardMap standard_map;
        TestMap test_map;

        {
            value_type first(1, 'a');
            value_type second(first);
            auto i1l = standard_map.insert(standard_map.cend(), std::move(first));
            auto i1r = test_map.insert(test_map.cend(), std::move(second));
            test_iterator_equal(i1l, i1r);
            test_map_equality<TestMap, StandardMap>(test_map, standard_map);
        }

        {
            value_type first(1, 'a');
            value_type second(first);
            auto i2l = standard_map.insert(standard_map.cend(), std::move(first));
            auto i2r = test_map.insert(test_map.cend(), std::move(second));
            test_iterator_equal(i2l, i2r);
            test_map_equality<TestMap, StandardMap>(test_map, standard_map);
        }

        {
            value_type first(1, 'c');
            value_type second(first);
            auto i3l = standard_map.insert(standard_map.cend(), std::move(first));
            auto i3r = test_map.insert(test_map.cend(), std::move(second));
            test_iterator_equal(i3l, i3r);
            test_map_equality<TestMap, StandardMap>(test_map, standard_map);
        }

        {
            value_type first(2, 'b');
            value_type second(first);
            auto i4l = standard_map.insert(standard_map.cend(), std::move(first));
            auto i4r = test_map.insert(test_map.cend(), std::move(second));
            test_iterator_equal(i4l, i4r);
            test_map_equality<TestMap, StandardMap>(test_map, standard_map);
        }

        {
            value_type first(2, 'b');
            value_type second(first);
            auto i5l = standard_map.insert(standard_map.cend(), std::move(first));
            auto i5r = test_map.insert(test_map.cend(), std::move(second));
            test_iterator_equal(i5l, i5r);
            test_map_equality<TestMap, StandardMap>(test_map, standard_map);
        }

        {
            value_type first(2, 'q');
            value_type second(first);
            auto i6l = standard_map.insert(standard_map.cend(), std::move(first));
            auto i6r = test_map.insert(test_map.cend(), std::move(second));
            test_iterator_equal(i6l, i6r);
            test_map_equality<TestMap, StandardMap>(test_map, standard_map);
        }
    }
}

template<typename T>
struct test_allocator : std::allocator<T>
{
    test_allocator() : valid(false) { }

    template<class U>
    test_allocator(const test_allocator<U>& other) : valid(other.valid) { }

    template<class U>
    struct rebind
    {
        typedef test_allocator<U> other;
    };

    bool valid;
};

void splay_map_test::testMiscellanneousOperations()
{
    {
        // Test allocator assignment
        using TestMap = bushy::splay_map<int, char, std::less<int>, test_allocator<std::pair<const int,char>>>;
        test_allocator<std::pair<const int,char>> alloc;
        alloc.valid = true;

        TestMap testMap(alloc);

        QVERIFY(testMap.get_allocator().valid);
    }
}

QTEST_MAIN(splay_map_test)

#include "tst_splay_map_test.moc"
