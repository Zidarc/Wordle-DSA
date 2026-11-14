#include "TestFramework.h"
#include "ds/AVLTree.h"
#include "ds/SafeArray.h"

#include <cstddef>
#include <string>

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

// Returns true if the SafeArray is sorted in strictly ascending order.
template <typename T>
bool isSorted(const SafeArray<T>& arr)
{
    for (std::size_t i = 1; i < arr.size(); ++i)
    {
        if (!(arr[i - 1] < arr[i]))
            return false;
    }
    return true;
}

// ===========================================================================
// Construction
// ===========================================================================

TEST_CASE(avlDefaultConstructedIsEmpty)
{
    AVLTree<int> tree;
    CHECK(tree.empty());
    CHECK_EQ(tree.size(), 0u);
    CHECK_EQ(tree.height(), 0);
}

// ===========================================================================
// Insertion and contains
// ===========================================================================

TEST_CASE(avlSingleInsert)
{
    AVLTree<int> tree;
    tree.insert(10);
    CHECK(!tree.empty());
    CHECK_EQ(tree.size(), 1u);
    CHECK_EQ(tree.height(), 1);
    CHECK(tree.contains(10));
    CHECK(!tree.contains(5));
}

TEST_CASE(avlMultipleInserts)
{
    AVLTree<int> tree;
    tree.insert(5);
    tree.insert(3);
    tree.insert(7);
    tree.insert(1);
    tree.insert(4);

    CHECK_EQ(tree.size(), 5u);
    CHECK(tree.contains(5));
    CHECK(tree.contains(3));
    CHECK(tree.contains(7));
    CHECK(tree.contains(1));
    CHECK(tree.contains(4));
    CHECK(!tree.contains(0));
    CHECK(!tree.contains(6));
    CHECK(!tree.contains(100));
}

TEST_CASE(avlDuplicateInsertIsIgnored)
{
    AVLTree<int> tree;
    tree.insert(10);
    tree.insert(10);
    tree.insert(10);
    CHECK_EQ(tree.size(), 1u);
    CHECK(tree.contains(10));
}

TEST_CASE(avlSearchMissingInEmptyTree)
{
    AVLTree<std::string> tree;
    CHECK(!tree.contains("CRANE"));
}

// ===========================================================================
// AVL rotations
// ===========================================================================

// LL case: inserting in strictly decreasing order forces right rotations.
//
//   Insert 30, 20, 10:
//
//       30            20
//      /       =>    /  \
//    20             10   30
//   /
//  10
//
TEST_CASE(avlLLRotation)
{
    AVLTree<int> tree;
    tree.insert(30);
    tree.insert(20);
    tree.insert(10);

    CHECK_EQ(tree.size(), 3u);
    CHECK_EQ(tree.height(), 2); // balanced: height 2, not 3

    CHECK(tree.contains(10));
    CHECK(tree.contains(20));
    CHECK(tree.contains(30));
}

// RR case: inserting in strictly increasing order forces left rotations.
//
//   Insert 10, 20, 30:
//
//   10               20
//     \       =>    /  \
//      20          10   30
//        \
//        30
//
TEST_CASE(avlRRRotation)
{
    AVLTree<int> tree;
    tree.insert(10);
    tree.insert(20);
    tree.insert(30);

    CHECK_EQ(tree.size(), 3u);
    CHECK_EQ(tree.height(), 2);

    CHECK(tree.contains(10));
    CHECK(tree.contains(20));
    CHECK(tree.contains(30));
}

// LR case: left child is right-heavy.
//
//   Insert 30, 10, 20:
//
//     30            20
//    /       =>    /  \
//   10            10   30
//     \
//      20
//
TEST_CASE(avlLRRotation)
{
    AVLTree<int> tree;
    tree.insert(30);
    tree.insert(10);
    tree.insert(20);

    CHECK_EQ(tree.size(), 3u);
    CHECK_EQ(tree.height(), 2);

    CHECK(tree.contains(10));
    CHECK(tree.contains(20));
    CHECK(tree.contains(30));
}

// RL case: right child is left-heavy.
//
//   Insert 10, 30, 20:
//
//   10              20
//     \      =>    /  \
//      30         10   30
//     /
//    20
//
TEST_CASE(avlRLRotation)
{
    AVLTree<int> tree;
    tree.insert(10);
    tree.insert(30);
    tree.insert(20);

    CHECK_EQ(tree.size(), 3u);
    CHECK_EQ(tree.height(), 2);

    CHECK(tree.contains(10));
    CHECK(tree.contains(20));
    CHECK(tree.contains(30));
}

// ===========================================================================
// Height stays O(log n) after sorted insertions
// ===========================================================================

TEST_CASE(avlHeightStaysLogarithmic)
{
    AVLTree<int> tree;

    // Insert 1000 values in sorted order. An unbalanced BST would reach
    // height 1000, but the AVL tree should stay around log2(1000) ~ 10.
    for (int i = 1; i <= 1000; ++i)
        tree.insert(i);

    CHECK_EQ(tree.size(), 1000u);

    // AVL height for 1000 nodes is at most ~1.44 * log2(1001) ~ 14.4.
    // We use 15 as a generous upper bound.
    CHECK(tree.height() <= 15);

    // Verify every value is still findable.
    for (int i = 1; i <= 1000; ++i)
        CHECK(tree.contains(i));

    // Verify missing values are not found.
    CHECK(!tree.contains(0));
    CHECK(!tree.contains(1001));
}

// ===========================================================================
// Iterative in-order traversal using Stack (simulation of recursion)
// ===========================================================================

TEST_CASE(avlInOrderTraversalReturnsSortedOutput)
{
    AVLTree<int> tree;
    tree.insert(50);
    tree.insert(30);
    tree.insert(70);
    tree.insert(10);
    tree.insert(40);
    tree.insert(60);
    tree.insert(80);

    SafeArray<int> sorted;
    tree.toSortedArray(sorted);

    CHECK_EQ(sorted.size(), 7u);
    CHECK(isSorted(sorted));

    // Verify exact values.
    CHECK_EQ(sorted[0], 10);
    CHECK_EQ(sorted[1], 30);
    CHECK_EQ(sorted[2], 40);
    CHECK_EQ(sorted[3], 50);
    CHECK_EQ(sorted[4], 60);
    CHECK_EQ(sorted[5], 70);
    CHECK_EQ(sorted[6], 80);
}

TEST_CASE(avlInOrderOnEmptyTree)
{
    AVLTree<int> tree;
    SafeArray<int> sorted;
    tree.toSortedArray(sorted);
    CHECK_EQ(sorted.size(), 0u);
}

TEST_CASE(avlInOrderOnSingleElement)
{
    AVLTree<int> tree;
    tree.insert(42);
    SafeArray<int> sorted;
    tree.toSortedArray(sorted);
    CHECK_EQ(sorted.size(), 1u);
    CHECK_EQ(sorted[0], 42);
}

TEST_CASE(avlInOrderAfterSortedInsertions)
{
    AVLTree<int> tree;
    for (int i = 1; i <= 100; ++i)
        tree.insert(i);

    SafeArray<int> sorted;
    tree.toSortedArray(sorted);

    CHECK_EQ(sorted.size(), 100u);
    CHECK(isSorted(sorted));
    CHECK_EQ(sorted[0], 1);
    CHECK_EQ(sorted[99], 100);
}

TEST_CASE(avlInOrderAfterReverseSortedInsertions)
{
    AVLTree<int> tree;
    for (int i = 100; i >= 1; --i)
        tree.insert(i);

    SafeArray<int> sorted;
    tree.toSortedArray(sorted);

    CHECK_EQ(sorted.size(), 100u);
    CHECK(isSorted(sorted));
    CHECK_EQ(sorted[0], 1);
    CHECK_EQ(sorted[99], 100);
}

// ===========================================================================
// Search on std::string (dictionary use case)
// ===========================================================================

TEST_CASE(avlContainsWithStrings)
{
    AVLTree<std::string> tree;
    tree.insert("CRANE");
    tree.insert("APPLE");
    tree.insert("SLATE");
    tree.insert("BRAVE");
    tree.insert("PLUMB");

    CHECK(tree.contains("CRANE"));
    CHECK(tree.contains("APPLE"));
    CHECK(tree.contains("SLATE"));
    CHECK(tree.contains("BRAVE"));
    CHECK(tree.contains("PLUMB"));
    CHECK(!tree.contains("ZEBRA"));
    CHECK(!tree.contains("ABACK"));

    SafeArray<std::string> sorted;
    tree.toSortedArray(sorted);
    CHECK_EQ(sorted.size(), 5u);
    CHECK(isSorted(sorted));
    CHECK_EQ(sorted[0], std::string("APPLE"));
    CHECK_EQ(sorted[4], std::string("SLATE"));
}

// ===========================================================================
// Comparison counter (DSA Insights)
// ===========================================================================

TEST_CASE(avlComparisonCounterIncrementsOnSearch)
{
    AVLTree<int> tree;
    for (int i = 1; i <= 100; ++i)
        tree.insert(i);

    int comparisons = 0;

    // Search for a value that exists.
    comparisons = 0;
    CHECK(tree.contains(50, comparisons));
    CHECK(comparisons > 0);

    // Search for a value that does not exist.
    comparisons = 0;
    CHECK(!tree.contains(999, comparisons));
    CHECK(comparisons > 0);
}

// ===========================================================================
// Clear
// ===========================================================================

TEST_CASE(avlClearEmptiesTheTree)
{
    AVLTree<int> tree;
    for (int i = 0; i < 50; ++i)
        tree.insert(i);
    CHECK_EQ(tree.size(), 50u);

    tree.clear();
    CHECK(tree.empty());
    CHECK_EQ(tree.size(), 0u);
    CHECK_EQ(tree.height(), 0);
    CHECK(!tree.contains(0));

    // Still usable after clear.
    tree.insert(42);
    CHECK(tree.contains(42));
    CHECK_EQ(tree.size(), 1u);
}

// ===========================================================================
// Rule of Three: deep copies
// ===========================================================================

TEST_CASE(avlCopyConstructorMakesDeepCopy)
{
    AVLTree<int> original;
    original.insert(10);
    original.insert(5);
    original.insert(15);
    original.insert(3);
    original.insert(7);

    AVLTree<int> copy(original);
    CHECK_EQ(copy.size(), 5u);
    CHECK(copy.contains(10));
    CHECK(copy.contains(3));
    CHECK(copy.contains(15));

    // Modifying the copy does not affect the original.
    copy.insert(20);
    copy.insert(1);
    CHECK_EQ(copy.size(), 7u);
    CHECK_EQ(original.size(), 5u);
    CHECK(!original.contains(20));

    // Both traversals should be independently sorted.
    SafeArray<int> origSorted;
    SafeArray<int> copySorted;
    original.toSortedArray(origSorted);
    copy.toSortedArray(copySorted);
    CHECK(isSorted(origSorted));
    CHECK(isSorted(copySorted));
    CHECK_EQ(origSorted.size(), 5u);
    CHECK_EQ(copySorted.size(), 7u);
}

TEST_CASE(avlCopyOfEmptyTreeIsEmpty)
{
    AVLTree<int> empty;
    AVLTree<int> copy(empty);
    CHECK(copy.empty());
    copy.insert(1);
    CHECK(empty.empty());
}

TEST_CASE(avlCopyAssignmentMakesDeepCopy)
{
    AVLTree<std::string> a;
    a.insert("APPLE");

    AVLTree<std::string> b;
    b.insert("X");
    b.insert("Y");
    b.insert("Z");

    b = a;
    CHECK_EQ(b.size(), 1u);
    CHECK(b.contains("APPLE"));
    CHECK(!b.contains("X"));

    // Modifying b does not affect a.
    b.insert("PLUMB");
    CHECK(!a.contains("PLUMB"));
    CHECK_EQ(a.size(), 1u);
}

TEST_CASE(avlSelfAssignmentLeavesTreeUnchanged)
{
    AVLTree<int> tree;
    tree.insert(4);
    tree.insert(8);
    tree.insert(2);

    AVLTree<int>& alias = tree;
    tree = alias;

    CHECK_EQ(tree.size(), 3u);
    CHECK(tree.contains(4));
    CHECK(tree.contains(8));
    CHECK(tree.contains(2));
}

int main()
{
    return tf::runAll("AVLTree");
}
