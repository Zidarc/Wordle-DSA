#include "TestFramework.h"
#include "algorithms/Sort.h"

#include <string>

struct Score
{
    int points;
    int order;
};

bool operator<(const Score& a, const Score& b)
{
    return a.points < b.points;
}

template <typename Arr>
bool isSorted(const Arr& a, int n)
{
    for (int i = 1; i < n; i++)
    {
        if (a[i] < a[i - 1])
            return false;
    }
    return true;
}

// ---------------------------------------------------------------------------
// Insertion sort
// ---------------------------------------------------------------------------

TEST_CASE(insertionSortEmptyAndOneElement)
{
    int empty[1] = {0};
    insertionSort(empty, 0);

    int one[] = {7};
    insertionSort(one, 1);
    CHECK_EQ(one[0], 7);
}

TEST_CASE(insertionSortSortsReversedAndSorted)
{
    int reversed[] = {5, 4, 3, 2, 1};
    insertionSort(reversed, 5);
    CHECK(isSorted(reversed, 5));
    CHECK_EQ(reversed[0], 1);
    CHECK_EQ(reversed[4], 5);

    int sorted[] = {1, 2, 3, 4, 5};
    insertionSort(sorted, 5);
    CHECK(isSorted(sorted, 5));
    CHECK_EQ(sorted[2], 3);
}

TEST_CASE(insertionSortKeepsDuplicates)
{
    int a[] = {3, 1, 3, 2, 1};
    insertionSort(a, 5);
    CHECK(isSorted(a, 5));
    CHECK_EQ(a[0], 1);
    CHECK_EQ(a[1], 1);
    CHECK_EQ(a[3], 3);
    CHECK_EQ(a[4], 3);
}

TEST_CASE(insertionSortAddsOneScoreToSortedList)
{
    // List is sorted, the new score is added at the end, then one pass fixes it.
    int scores[] = {100, 250, 400, 175};
    insertionSort(scores, 4);
    CHECK_EQ(scores[0], 100);
    CHECK_EQ(scores[1], 175);
    CHECK_EQ(scores[2], 250);
    CHECK_EQ(scores[3], 400);
}

// ---------------------------------------------------------------------------
// Merge sort
// ---------------------------------------------------------------------------

TEST_CASE(mergeSortEmptyAndOneElement)
{
    int empty[1] = {0};
    mergeSort(empty, 0);

    int one[] = {7};
    mergeSort(one, 1);
    CHECK_EQ(one[0], 7);
}

TEST_CASE(mergeSortSortsReversedAndSorted)
{
    int reversed[] = {9, 8, 7, 6, 5, 4, 3, 2, 1};
    mergeSort(reversed, 9);
    CHECK(isSorted(reversed, 9));
    CHECK_EQ(reversed[0], 1);
    CHECK_EQ(reversed[8], 9);

    int sorted[] = {1, 2, 3, 4};
    mergeSort(sorted, 4);
    CHECK(isSorted(sorted, 4));
}

TEST_CASE(mergeSortKeepsDuplicates)
{
    int a[] = {4, 2, 4, 1, 2, 4};
    mergeSort(a, 6);
    CHECK(isSorted(a, 6));
    CHECK_EQ(a[0], 1);
    CHECK_EQ(a[1], 2);
    CHECK_EQ(a[2], 2);
    CHECK_EQ(a[3], 4);
    CHECK_EQ(a[5], 4);
}

TEST_CASE(mergeSortSortsStrings)
{
    std::string words[] = {"SLATE", "APPLE", "PLUMB", "CRANE", "BRAVE"};
    mergeSort(words, 5);
    CHECK(isSorted(words, 5));
    CHECK_EQ(words[0], std::string("APPLE"));
    CHECK_EQ(words[4], std::string("SLATE"));
}

TEST_CASE(mergeSortIsStable)
{
    // Equal scores must keep their original order.
    Score entries[] = {{5, 0}, {3, 1}, {5, 2}, {3, 3}, {1, 4}};
    mergeSort(entries, 5);

    CHECK_EQ(entries[0].points, 1);
    CHECK_EQ(entries[1].points, 3);
    CHECK_EQ(entries[2].points, 3);
    CHECK_EQ(entries[3].points, 5);
    CHECK_EQ(entries[4].points, 5);

    CHECK_EQ(entries[0].order, 4);
    CHECK_EQ(entries[1].order, 1);
    CHECK_EQ(entries[2].order, 3);
    CHECK_EQ(entries[3].order, 0);
    CHECK_EQ(entries[4].order, 2);
}

// ---------------------------------------------------------------------------
// Radix sort
// ---------------------------------------------------------------------------

TEST_CASE(radixSortEmptyAndOneWord)
{
    std::string empty[1] = {"AAAAA"};
    radixSortWords(empty, 0);

    std::string one[] = {"CRANE"};
    radixSortWords(one, 1);
    CHECK_EQ(one[0], std::string("CRANE"));
}

TEST_CASE(radixSortSortsReversedAndSorted)
{
    std::string reversed[] = {"SLATE", "PLUMB", "GRAPE", "CRANE", "APPLE"};
    radixSortWords(reversed, 5);
    CHECK(isSorted(reversed, 5));
    CHECK_EQ(reversed[0], std::string("APPLE"));
    CHECK_EQ(reversed[1], std::string("CRANE"));
    CHECK_EQ(reversed[4], std::string("SLATE"));

    std::string sorted[] = {"APPLE", "BRAVE", "CRANE"};
    radixSortWords(sorted, 3);
    CHECK(isSorted(sorted, 3));
}

TEST_CASE(radixSortHandlesDuplicatesAndSharedPrefixes)
{
    std::string words[] = {"CRANE", "CRATE", "CRANE", "CRAZE", "CRABS"};
    radixSortWords(words, 5);
    CHECK(isSorted(words, 5));
    CHECK_EQ(words[0], std::string("CRABS"));
    CHECK_EQ(words[1], std::string("CRANE"));
    CHECK_EQ(words[2], std::string("CRANE"));
    CHECK_EQ(words[3], std::string("CRATE"));
    CHECK_EQ(words[4], std::string("CRAZE"));
}

TEST_CASE(radixSortMatchesMergeSort)
{
    std::string a[] = {"TRAIN", "ABOUT", "WORLD", "ZEBRA", "MOUSE", "APPLE", "QUIET", "BRAVE"};
    std::string b[] = {"TRAIN", "ABOUT", "WORLD", "ZEBRA", "MOUSE", "APPLE", "QUIET", "BRAVE"};
    radixSortWords(a, 8);
    mergeSort(b, 8);
    for (int i = 0; i < 8; i++)
        CHECK_EQ(a[i], b[i]);
}

int main()
{
    return tf::runAll("Sort");
}