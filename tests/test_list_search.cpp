// Tests for linear and binary search on a singly linked list.

#include "TestFramework.h"

#include "algorithms/ListSearch.h"

#include <string>

static SinglyLinkedList<int> evens(int n)
{
    SinglyLinkedList<int> list;
    for (int i = 0; i < n; ++i)
        list.pushBack(i * 2);
    return list;
}

TEST_CASE(emptyListFindsNothing)
{
    SinglyLinkedList<int> list;
    CHECK_EQ(linearSearchList(list, 3), -1);
    CHECK_EQ(binarySearchList(list, 3), -1);
}

TEST_CASE(findsFirstMiddleAndLast)
{
    SinglyLinkedList<int> list = evens(9); // 0 2 4 ... 16
    CHECK_EQ(linearSearchList(list, 0), 0);
    CHECK_EQ(linearSearchList(list, 8), 4);
    CHECK_EQ(linearSearchList(list, 16), 8);
    CHECK_EQ(binarySearchList(list, 0), 0);
    CHECK_EQ(binarySearchList(list, 8), 4);
    CHECK_EQ(binarySearchList(list, 16), 8);
}

TEST_CASE(missingValuesBetweenElements)
{
    SinglyLinkedList<int> list = evens(10);
    CHECK_EQ(binarySearchList(list, -1), -1);
    CHECK_EQ(binarySearchList(list, 7), -1);
    CHECK_EQ(binarySearchList(list, 99), -1);
    CHECK_EQ(linearSearchList(list, 7), -1);
}

TEST_CASE(binaryAndLinearAgreeForEverySize)
{
    bool agree = true;
    for (int n = 0; n <= 33; ++n)
    {
        SinglyLinkedList<int> list = evens(n);
        for (int key = -1; key <= 2 * n; ++key)
            agree = agree && binarySearchList(list, key) == linearSearchList(list, key);
    }
    CHECK(agree);
}

TEST_CASE(binaryUsesFewComparisonsButManyPointerMoves)
{
    SinglyLinkedList<int> list = evens(1024);
    ListSearchStats linear;
    ListSearchStats binary;
    CHECK_EQ(linearSearchList(list, 2046, &linear), 1023);
    CHECK_EQ(binarySearchList(list, 2046, &binary), 1023);

    CHECK_EQ(linear.comparisons, 1024);
    CHECK(binary.comparisons <= 11);     // O(log n) comparisons
    CHECK(binary.pointerMoves >= 1024);  // but O(n) walking to find middles
}

TEST_CASE(worksOnWords)
{
    SinglyLinkedList<std::string> words;
    const char* sorted[] = {"APPLE", "BRAVE", "CRANE", "GHOST", "PLUMB", "SLATE"};
    for (const char* w : sorted)
        words.pushBack(w);
    CHECK_EQ(binarySearchList(words, std::string("GHOST")), 3);
    CHECK_EQ(binarySearchList(words, std::string("ZEBRA")), -1);
    CHECK_EQ(linearSearchList(words, std::string("APPLE")), 0);
}

int main()
{
    return tf::runAll("Linked list search");
}
