#include "TestFramework.h"
#include "ds/BST.h"

#include <cstddef>
#include <string>

// ===========================================================================
// Construction
// ===========================================================================

TEST_CASE(bstDefaultConstructedIsEmpty)
{
    BST<int> tree;
    CHECK(tree.empty());
    CHECK_EQ(tree.size(), 0u);
    CHECK_EQ(tree.height(), 0);
}

// ===========================================================================
// Insertion and contains
// ===========================================================================

TEST_CASE(bstSingleInsert)
{
    BST<int> tree;
    tree.insert(10);
    CHECK(!tree.empty());
    CHECK_EQ(tree.size(), 1u);
    CHECK_EQ(tree.height(), 1);
    CHECK(tree.contains(10));
    CHECK(!tree.contains(5));
}

TEST_CASE(bstMultipleInserts)
{
    BST<int> tree;
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

TEST_CASE(bstDuplicateInsertIsIgnored)
{
    BST<int> tree;
    tree.insert(10);
    tree.insert(10);
    tree.insert(10);
    CHECK_EQ(tree.size(), 1u);
    CHECK(tree.contains(10));
}

TEST_CASE(bstSearchMissingInEmptyTree)
{
    BST<std::string> tree;
    CHECK(!tree.contains("CRANE"));
}

// ===========================================================================
// Search on char (secret-word use case)
// ===========================================================================

TEST_CASE(bstSecretWordLetters)
{
    // Secret word: CRANE -> letters C, R, A, N, E
    BST<char> tree;
    tree.insert('C');
    tree.insert('R');
    tree.insert('A');
    tree.insert('N');
    tree.insert('E');

    CHECK_EQ(tree.size(), 5u);

    // Letters in the word
    CHECK(tree.contains('C'));
    CHECK(tree.contains('R'));
    CHECK(tree.contains('A'));
    CHECK(tree.contains('N'));
    CHECK(tree.contains('E'));

    // Letters not in the word
    CHECK(!tree.contains('B'));
    CHECK(!tree.contains('Z'));
    CHECK(!tree.contains('X'));
}

// ===========================================================================
// Comparison counter (DSA Insights)
// ===========================================================================

TEST_CASE(bstComparisonCounterIncrementsOnSearch)
{
    BST<int> tree;
    tree.insert(10);
    tree.insert(5);
    tree.insert(15);
    tree.insert(3);
    tree.insert(7);

    int comparisons = 0;

    // Search for root (10): should find it quickly.
    comparisons = 0;
    CHECK(tree.contains(10, comparisons));
    CHECK(comparisons > 0);

    // Search for a leaf (3): should need more comparisons.
    int rootComps = comparisons;
    comparisons = 0;
    CHECK(tree.contains(3, comparisons));
    CHECK(comparisons > rootComps);

    // Search for a missing value.
    comparisons = 0;
    CHECK(!tree.contains(100, comparisons));
    CHECK(comparisons > 0);
}

// ===========================================================================
// Search on std::string
// ===========================================================================

TEST_CASE(bstContainsWithStrings)
{
    BST<std::string> tree;
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
}

// ===========================================================================
// Height
// ===========================================================================

TEST_CASE(bstHeightReflectsStructure)
{
    BST<int> tree;
    CHECK_EQ(tree.height(), 0);

    tree.insert(10);
    CHECK_EQ(tree.height(), 1);

    tree.insert(5);
    CHECK_EQ(tree.height(), 2);

    tree.insert(15);
    CHECK_EQ(tree.height(), 2);

    // Inserting sorted data into an unbalanced BST makes it degenerate.
    BST<int> degen;
    for (int i = 1; i <= 10; ++i)
        degen.insert(i);
    CHECK_EQ(degen.height(), 10);
}

// ===========================================================================
// Clear
// ===========================================================================

TEST_CASE(bstClearEmptiesTheTree)
{
    BST<int> tree;
    for (int i = 0; i < 20; ++i)
        tree.insert(i);
    CHECK_EQ(tree.size(), 20u);

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

TEST_CASE(bstCopyConstructorMakesDeepCopy)
{
    BST<int> original;
    original.insert(10);
    original.insert(5);
    original.insert(15);

    BST<int> copy(original);
    CHECK_EQ(copy.size(), 3u);
    CHECK(copy.contains(10));
    CHECK(copy.contains(5));
    CHECK(copy.contains(15));

    // Modifying the copy does not affect the original.
    copy.insert(20);
    copy.insert(1);
    CHECK_EQ(copy.size(), 5u);
    CHECK_EQ(original.size(), 3u);
    CHECK(!original.contains(20));
}

TEST_CASE(bstCopyOfEmptyTreeIsEmpty)
{
    BST<int> empty;
    BST<int> copy(empty);
    CHECK(copy.empty());
    copy.insert(1);
    CHECK(empty.empty());
}

TEST_CASE(bstCopyAssignmentMakesDeepCopy)
{
    BST<std::string> a;
    a.insert("APPLE");

    BST<std::string> b;
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

TEST_CASE(bstSelfAssignmentLeavesTreeUnchanged)
{
    BST<int> tree;
    tree.insert(4);
    tree.insert(8);
    tree.insert(2);

    BST<int>& alias = tree;
    tree = alias;

    CHECK_EQ(tree.size(), 3u);
    CHECK(tree.contains(4));
    CHECK(tree.contains(8));
    CHECK(tree.contains(2));
}

int main()
{
    return tf::runAll("BST");
}
