#ifndef MAPTESTALGORITHMS_H
#define MAPTESTALGORITHMS_H

#include <QtTest>

#include <algorithm>

template<typename It1, typename It2>
bool is_range_equals(It1 b1, It1 e1, It2 b2, It2 e2)
{
    if (std::distance(b1, e1) != std::distance(b2, e2))
    {
        return false;
    }

    return std::equal(b1, e1, b2, e2);
}

template<typename Map1, typename Map2>
void test_map_equality(const Map1& map1, const Map2& map2)
{
    static_assert(std::is_same<typename Map1::key_type, typename Map2::key_type>::value, "Map type differs!");
    static_assert(std::is_same<typename Map1::mapped_type, typename Map2::mapped_type>::value, "Map type differs!");
    static_assert(std::is_same<typename Map1::value_type, typename Map2::value_type>::value, "Map type differs!");
    static_assert(std::is_same<typename Map1::key_compare, typename Map2::key_compare>::value, "Map type differs!");
    static_assert(std::is_same<typename Map1::reference, typename Map2::reference>::value, "Map type differs!");
    static_assert(std::is_same<typename Map1::const_reference, typename Map2::const_reference>::value, "Map type differs!");
    static_assert(std::is_same<typename Map1::pointer, typename Map2::pointer>::value, "Map type differs!");
    static_assert(std::is_same<typename Map1::const_pointer, typename Map2::const_pointer>::value, "Map type differs!");
    static_assert(std::is_same<typename Map1::difference_type, typename Map2::difference_type>::value, "Map type differs!");
    static_assert(std::is_same<typename Map1::size_type, typename Map2::size_type>::value, "Map type differs!");

    QVERIFY(map1.size() == map2.size());
    QVERIFY(map1.empty() == map2.empty());

    QVERIFY(is_range_equals(map1.begin(), map1.end(), map2.begin(), map2.end()));
    QVERIFY(is_range_equals(map1.cbegin(), map1.cend(), map2.cbegin(), map2.cend()));
    QVERIFY(is_range_equals(map1.rbegin(), map1.rend(), map2.rbegin(), map2.rend()));
    QVERIFY(is_range_equals(map1.crbegin(), map1.crend(), map2.crbegin(), map2.crend()));
}

template<typename Iterator1, typename Iterator2>
void test_operation_result_equal(const std::pair<Iterator1, bool>& left, const std::pair<Iterator2, bool>& right)
{
    QVERIFY(left.second == right.second);
    QVERIFY(*left.first == *right.first);
}

template<typename Iterator1, typename Iterator2>
void test_iterator_equal(Iterator1 left, Iterator2 right)
{
    QVERIFY(*left == *right);
    QVERIFY(left->first == right->first);
    QVERIFY(left->second == right->second);
}

#endif // MAPTESTALGORITHMS_H
