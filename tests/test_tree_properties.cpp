// Tests for the binary tree properties on BST and AVLTree (lab week 10).

#include "TestFramework.h"

#include "ds/AVLTree.h"
#include "ds/BST.h"

#include <string>

static void insertAll(BST<int>& t, const int* values, int n)
{
    for (int i = 0; i < n; ++i)
        t.insert(values[i]);
}

TEST_CASE(emptyTreeIsFullCompleteAndPerfect)
{
    BST<int> t;
    CHECK(t.isFull());
    CHECK(t.isComplete());
    CHECK(t.isPerfect());
    CHECK_EQ(t.leafCount(), 0u);
}

TEST_CASE(singleNodeIsPerfect)
{
    BST<int> t;
    t.insert(5);
    CHECK(t.isPerfect());
    CHECK_EQ(t.leafCount(), 1u);
}

TEST_CASE(perfectTreeOfSeven)
{
    BST<int> t;
    int v[] = {4, 2, 6, 1, 3, 5, 7};
    insertAll(t, v, 7);
    TreeShape s = t.shape();
    CHECK_EQ(s.nodes, 7u);
    CHECK_EQ(s.leaves, 4u);
    CHECK_EQ(s.internalNodes, 3u);
    CHECK_EQ(s.height, 3);
    CHECK_EQ(s.minPossibleHeight, 3);
    CHECK(s.full);
    CHECK(s.complete);
    CHECK(s.perfect);
}

TEST_CASE(fullButNotComplete)
{
    //     2
    //    / \     (diagram)
    //   1   4
    //      / \   (diagram)
    //     3   5
    BST<int> t;
    int v[] = {2, 1, 4, 3, 5};
    insertAll(t, v, 5);
    CHECK(t.isFull());
    CHECK(!t.isComplete());
    CHECK(!t.isPerfect());
}

TEST_CASE(completeButNotFull)
{
    //       3
    //      / \   (diagram)
    //     2   4
    //    /
    //   1
    BST<int> t;
    int v[] = {3, 2, 4, 1};
    insertAll(t, v, 4);
    CHECK(!t.isFull());
    CHECK(t.isComplete());
    CHECK_EQ(t.shape().singleChildNodes, 1u);
}

TEST_CASE(gapOnLastLevelIsNotComplete)
{
    //       3
    //      / \   (diagram)
    //     1   4
    //      \     (diagram)
    //       2
    BST<int> t;
    int v[] = {3, 1, 4, 2};
    insertAll(t, v, 4);
    CHECK(!t.isComplete());
}

TEST_CASE(sortedInsertDegeneratesBstButNotAvl)
{
    BST<int> bst;
    AVLTree<int> avl;
    for (int i = 1; i <= 63; ++i)
    {
        bst.insert(i);
        avl.insert(i);
    }
    CHECK_EQ(bst.shape().height, 63); // a linked list in disguise
    CHECK_EQ(bst.leafCount(), 1u);
    CHECK_EQ(avl.shape().minPossibleHeight, 6);
    CHECK(avl.height() <= 7);
    CHECK(avl.isBalanced());
}

TEST_CASE(avlOfSevenSortedValuesIsPerfect)
{
    AVLTree<int> t;
    for (int i = 1; i <= 7; ++i)
        t.insert(i);
    CHECK(t.isPerfect());
    CHECK(t.isFull());
    CHECK(t.isComplete());
    CHECK_EQ(t.leafCount(), 4u);
}

TEST_CASE(avlShapeMatchesSize)
{
    AVLTree<std::string> t;
    const char* words[] = {"CRANE", "SLATE", "APPLE", "BRAVE", "AUDIO", "PLUMB", "GHOST", "TRAIN"};
    for (const char* w : words)
        t.insert(w);
    TreeShape s = t.shape();
    CHECK_EQ(s.nodes, t.size());
    CHECK_EQ(s.height, t.height());
    CHECK_EQ(s.leaves + s.internalNodes, s.nodes);
    CHECK(t.isBalanced());
}

TEST_CASE(avlStaysBalancedOnLargeInput)
{
    AVLTree<int> t;
    for (int i = 0; i < 2000; ++i)
        t.insert((i * 7919) % 2003);
    CHECK(t.isBalanced());
    CHECK(t.height() <= 16); // 1.44 log2(n) bound
}

int main()
{
    return tf::runAll("Binary tree properties");
}
