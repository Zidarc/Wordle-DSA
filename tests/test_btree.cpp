// Tests for BTree<T, Order> (2-3 tree by default).

#include "TestFramework.h"

#include "ds/BTree.h"
#include "ds/SafeArray.h"

#include <string>

TEST_CASE(emptyTree)
{
    BTree<int> t;
    CHECK(t.empty());
    CHECK_EQ(t.height(), 0);
    CHECK(!t.contains(1));
    CHECK(t.isValid());
}

TEST_CASE(rootSplitsOnThirdKey)
{
    BTree<int> t; // 2-3 tree: at most 2 keys per node
    t.insert(10);
    t.insert(20);
    CHECK_EQ(t.height(), 1);
    CHECK_EQ(t.nodeCount(), 1u);
    t.insert(30); // [10 20 30] splits: 20 moves up
    CHECK_EQ(t.height(), 2);
    CHECK_EQ(t.nodeCount(), 3u);
    CHECK(t.isValid());
}

TEST_CASE(duplicatesAreIgnored)
{
    BTree<std::string> t;
    CHECK(t.insert("CRANE"));
    CHECK(!t.insert("CRANE"));
    CHECK_EQ(t.size(), 1u);
}

TEST_CASE(sortedInsertKeepsAllLeavesOnOneLevel)
{
    BTree<int> t;
    for (int i = 1; i <= 1000; ++i)
        t.insert(i);
    CHECK(t.isValid());
    CHECK_EQ(t.size(), 1000u);
    CHECK(t.height() <= 10); // log2(1000) ~ 10, the worst case for a 2-3 tree
    for (int i = 1; i <= 1000; ++i)
        CHECK(t.contains(i));
    CHECK(!t.contains(0));
    CHECK(!t.contains(1001));
}

TEST_CASE(higherOrderGivesShorterTree)
{
    BTree<int, 3> twoThree;
    BTree<int, 5> order5;
    BTree<int, 16> order16;
    for (int i = 0; i < 2000; ++i)
    {
        int v = (i * 7919) % 2003;
        twoThree.insert(v);
        order5.insert(v);
        order16.insert(v);
    }
    CHECK(twoThree.isValid());
    CHECK(order5.isValid());
    CHECK(order16.isValid());
    CHECK(order5.height() < twoThree.height());
    CHECK(order16.height() < order5.height());
}

TEST_CASE(inOrderWalkIsSorted)
{
    BTree<int> t;
    int v[] = {50, 20, 80, 10, 30, 70, 90, 60, 40, 5, 95, 25};
    for (int x : v)
        t.insert(x);
    SafeArray<int> out;
    t.toSortedArray(out);
    CHECK_EQ(out.size(), 12u);
    bool sorted = true;
    for (std::size_t i = 1; i < out.size(); ++i)
        sorted = sorted && out[i - 1] < out[i];
    CHECK(sorted);
    CHECK_EQ(out[0], 5);
    CHECK_EQ(out[11], 95);
}

TEST_CASE(searchCountsComparisonsAndNodes)
{
    BTree<int> t;
    for (int i = 1; i <= 100; ++i)
        t.insert(i);
    int comparisons = 0;
    int nodes = 0;
    CHECK(t.search(57, comparisons, nodes));
    CHECK(nodes >= 1);
    CHECK(nodes <= t.height());
    CHECK(comparisons >= nodes);

    int missComparisons = 0;
    int missNodes = 0;
    CHECK(!t.search(1000, missComparisons, missNodes));
    CHECK_EQ(missNodes, t.height()); // a miss always reaches a leaf
}

TEST_CASE(copyIsDeep)
{
    BTree<int> a;
    for (int i = 0; i < 20; ++i)
        a.insert(i);

    BTree<int> b(a);
    b.insert(100);
    CHECK(!a.contains(100));
    CHECK(b.contains(100));
    CHECK(b.isValid());

    BTree<int> c;
    c.insert(-1);
    c = a;
    CHECK(!c.contains(-1));
    CHECK_EQ(c.size(), 20u);
    CHECK(c.isValid());

    a.clear();
    CHECK(c.contains(5));
    CHECK(a.empty());
}

int main()
{
    return tf::runAll("BTree");
}
