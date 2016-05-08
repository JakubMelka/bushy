
#include "../Bushy/include/splay_map.h"

#include <QString>
#include <QtTest>
#include <QCoreApplication>

#include <map>
#include <random>

class MapBenchmark : public QObject
{
    Q_OBJECT

public:
    MapBenchmark();

private Q_SLOTS:
    void testInsertFindDeleteUniform_data();
    void testInsertFindDeleteUniform();

    void testFindUniform_data();
    void testFindUniform();

    void testFindBinomialDistribution_data();
    void testFindBinomialDistribution();

    void testFindGeometricDistribution_data();
    void testFindGeometricDistribution();

private:
    enum EMapType : int
    {
        E_STL_MAP,
        E_SPLAY_MAP,
        E_SPLAY_MAP_CLASSIC
    };

    template<typename Map>
    void testInsertFindDeleteUniform_impl(int size);

    template<typename Map>
    void testFindUniform_impl(int size);

    template<typename Map>
    void testFindBinomialDistribution_impl(int size);

    template<typename Map>
    void testFindGeometricDistribution_impl(int size);
};

MapBenchmark::MapBenchmark()
{
}

void MapBenchmark::testInsertFindDeleteUniform_data()
{
    QTest::addColumn<int>("map_type");
    QTest::addColumn<int>("size");

    for (const int i : { 10, 100, 1000, 10000, 100000, 1000000})
    {
        QByteArray size = QByteArray::number(i);
        QTest::newRow("STL Map (" + size + " elements)") << (int)E_STL_MAP << i;
        QTest::newRow("Splay Map (" + size + " elements)") << (int)E_SPLAY_MAP << i;
        QTest::newRow("Splay Map Classic (" + size + " elements)") << (int)E_SPLAY_MAP_CLASSIC << i;
    }
}

void MapBenchmark::testInsertFindDeleteUniform()
{
    QFETCH(int, map_type);
    QFETCH(int, size);

    switch (map_type)
    {
        case E_STL_MAP:
            testInsertFindDeleteUniform_impl<std::map<int, int>>(size);
            break;

        case E_SPLAY_MAP:
            testInsertFindDeleteUniform_impl<bushy::splay_map<int, int>>(size);
            break;

        case E_SPLAY_MAP_CLASSIC:
            testInsertFindDeleteUniform_impl<bushy::splay_classic_map<int, int>>(size);
            break;

        default:
            QVERIFY2(false, "Unknown map type!");
            break;
    }
}

template<typename Map>
void MapBenchmark::testInsertFindDeleteUniform_impl(int size)
{
    Map map;

    // Prepare the test data
    std::vector<int> data(size);
    std::iota(data.begin(), data.end(), 1);
    std::random_shuffle(data.begin(), data.end());

    QBENCHMARK {

        // Insert the data
        for (const int value : data)
        {
            map.insert(std::make_pair(value, value * 37));
        }

        // Find all data
        std::random_shuffle(data.begin(), data.end());
        for (const int value : data)
        {
            volatile auto it = map.find(value);
        }

        // Delete all data
        for (const int value : data)
        {
            map.erase(value);
        }
    }
}

void MapBenchmark::testFindUniform_data()
{
    QTest::addColumn<int>("map_type");
    QTest::addColumn<int>("size");

    for (const int i : { 10, 100, 1000, 10000, 100000, 1000000})
    {
        QByteArray size = QByteArray::number(i);
        QTest::newRow("STL Map (" + size + " elements)") << (int)E_STL_MAP << i;
        QTest::newRow("Splay Map (" + size + " elements)") << (int)E_SPLAY_MAP << i;
        QTest::newRow("Splay Map Classic (" + size + " elements)") << (int)E_SPLAY_MAP_CLASSIC << i;
    }
}

void MapBenchmark::testFindUniform()
{
    QFETCH(int, map_type);
    QFETCH(int, size);

    switch (map_type)
    {
        case E_STL_MAP:
            testFindUniform_impl<std::map<int, int>>(size);
            break;

        case E_SPLAY_MAP:
            testFindUniform_impl<bushy::splay_map<int, int>>(size);
            break;

        case E_SPLAY_MAP_CLASSIC:
            testFindUniform_impl<bushy::splay_classic_map<int, int>>(size);
            break;

        default:
            QVERIFY2(false, "Unknown map type!");
            break;
    }
}

template<typename Map>
void MapBenchmark::testFindUniform_impl(int size)
{
    Map map;

    // Prepare the test data
    std::vector<int> data(size);
    std::iota(data.begin(), data.end(), 1);
    std::random_shuffle(data.begin(), data.end());

    // Insert the data
    for (const int value : data)
    {
        map.insert(std::make_pair(value, value * 37));
    }

    QBENCHMARK {
        // Find all data
        std::random_shuffle(data.begin(), data.end());
        for (const int value : data)
        {
            volatile auto it = map.find(value);
        }
    }

    // Delete all data
    for (const int value : data)
    {
        map.erase(value);
    }
}

void MapBenchmark::testFindBinomialDistribution_data()
{
    QTest::addColumn<int>("map_type");
    QTest::addColumn<int>("size");

    for (const int i : { 10, 100, 1000, 10000, 100000, 1000000})
    {
        QByteArray size = QByteArray::number(i);
        QTest::newRow("STL Map (" + size + " elements)") << (int)E_STL_MAP << i;
        QTest::newRow("Splay Map (" + size + " elements)") << (int)E_SPLAY_MAP << i;
        QTest::newRow("Splay Map Classic (" + size + " elements)") << (int)E_SPLAY_MAP_CLASSIC << i;
    }
}

void MapBenchmark::testFindBinomialDistribution()
{
    QFETCH(int, map_type);
    QFETCH(int, size);

    switch (map_type)
    {
        case E_STL_MAP:
            testFindBinomialDistribution_impl<std::map<int, int>>(size);
            break;

        case E_SPLAY_MAP:
            testFindBinomialDistribution_impl<bushy::splay_map<int, int>>(size);
            break;

        case E_SPLAY_MAP_CLASSIC:
            testFindBinomialDistribution_impl<bushy::splay_classic_map<int, int>>(size);
            break;

        default:
            QVERIFY2(false, "Unknown map type!");
            break;
    }
}

template<typename Map>
void MapBenchmark::testFindBinomialDistribution_impl(int size)
{
    Map map;

    // Prepare the test data
    std::vector<int> data(size);
    std::iota(data.begin(), data.end(), 0);
    std::random_shuffle(data.begin(), data.end());

    // Insert the data
    for (const int value : data)
    {
        map.insert(std::make_pair(value, value * 37));
    }

    std::minstd_rand engine(0);
    std::binomial_distribution<int> distribution(size - 1, 0.5);

    QBENCHMARK {
        // Find all data
        for (int i = 0; i < size; ++i)
        {
            const int value = distribution(engine);
            volatile auto it = map.find(value);
        }
    }

    // Delete all data
    for (const int value : data)
    {
        map.erase(value);
    }
}

void MapBenchmark::testFindGeometricDistribution_data()
{
    QTest::addColumn<int>("map_type");
    QTest::addColumn<int>("size");

    for (const int i : { 10, 100, 1000, 10000, 100000, 1000000})
    {
        QByteArray size = QByteArray::number(i);
        QTest::newRow("STL Map (" + size + " elements)") << (int)E_STL_MAP << i;
        QTest::newRow("Splay Map (" + size + " elements)") << (int)E_SPLAY_MAP << i;
        QTest::newRow("Splay Map Classic (" + size + " elements)") << (int)E_SPLAY_MAP_CLASSIC << i;
    }
}

void MapBenchmark::testFindGeometricDistribution()
{
    QFETCH(int, map_type);
    QFETCH(int, size);

    switch (map_type)
    {
        case E_STL_MAP:
            testFindGeometricDistribution_impl<std::map<int, int>>(size);
            break;

        case E_SPLAY_MAP:
            testFindGeometricDistribution_impl<bushy::splay_map<int, int>>(size);
            break;

        case E_SPLAY_MAP_CLASSIC:
            testFindGeometricDistribution_impl<bushy::splay_classic_map<int, int>>(size);
            break;

        default:
            QVERIFY2(false, "Unknown map type!");
            break;
    }
}

template<typename Map>
void MapBenchmark::testFindGeometricDistribution_impl(int size)
{
    Map map;

    // Prepare the test data
    std::vector<int> data(size);
    std::iota(data.begin(), data.end(), 0);
    std::random_shuffle(data.begin(), data.end());

    // Insert the data
    for (const int value : data)
    {
        map.insert(std::make_pair(value, value * 37));
    }

    std::minstd_rand engine(0);
    std::geometric_distribution<int> distribution(0.5);

    QBENCHMARK {
        // Find all data
        for (int i = 0; i < size; ++i)
        {
            const int value = distribution(engine);
            volatile auto it = map.find(value);
        }
    }

    // Delete all data
    for (const int value : data)
    {
        map.erase(value);
    }
}

QTEST_MAIN(MapBenchmark)

#include "tst_MapBenchmark.moc"
