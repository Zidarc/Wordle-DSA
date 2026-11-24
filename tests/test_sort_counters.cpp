// Tests for bubble, selection, comb and quick sort, and for the SortCounter
// numbers the Sorting Lab reports.

#include "TestFramework.h"
#include "algorithms/Sort.h"

#include <string>

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

// Runs one sort on reversed, sorted, duplicate-heavy and tiny inputs.
template <typename SortFn>
void checkSortsCommonCases(SortFn sortFn)
{
    int empty[1] = {42};
    sortFn(empty, 0);
    CHECK_EQ(empty[0], 42);

    int one[] = {7};
    sortFn(one, 1);
    CHECK_EQ(one[0], 7);

    int reversed[] = {9, 8, 7, 6, 5, 4, 3, 2, 1, 0};
    sortFn(reversed, 10);
    CHECK(isSorted(reversed, 10));
    CHECK_EQ(reversed[0], 0);
    CHECK_EQ(reversed[9], 9);

    int sorted[] = {1, 2, 3, 4, 5, 6};
    sortFn(sorted, 6);
    CHECK(isSorted(sorted, 6));

    int dups[] = {3, 1, 3, 2, 1, 3, 2, 2};
    sortFn(dups, 8);
    CHECK(isSorted(dups, 8));
    CHECK_EQ(dups[0], 1);
    CHECK_EQ(dups[7], 3);

    std::string words[] = {"SLATE", "CRANE", "APPLE", "BRAVE", "AUDIO"};
    sortFn(words, 5);
    CHECK(isSorted(words, 5));
    CHECK_EQ(words[0], std::string("APPLE"));
    CHECK_EQ(words[4], std::string("SLATE"));
}

TEST_CASE(bubbleSortCommonCases)
{
    checkSortsCommonCases([](auto& a, int n) { bubbleSort(a, n); });
}

TEST_CASE(selectionSortCommonCases)
{
    checkSortsCommonCases([](auto& a, int n) { selectionSort(a, n); });
}

TEST_CASE(combSortCommonCases)
{
    checkSortsCommonCases([](auto& a, int n) { combSort(a, n); });
}

TEST_CASE(quickSortCommonCases)
{
    checkSortsCommonCases([](auto& a, int n) { quickSort(a, n); });
}

TEST_CASE(quickSortIterativeCommonCases)
{
    checkSortsCommonCases([](auto& a, int n) { quickSortIterative(a, n); });
}

TEST_CASE(bubbleSortStopsEarlyOnSortedInput)
{
    int a[] = {1, 2, 3, 4, 5, 6, 7, 8};
    SortCounter c;
    bubbleSort(a, 8, &c);
    CHECK_EQ(c.comparisons, 7LL); // one pass, no swaps
    CHECK_EQ(c.swaps, 0LL);
}

TEST_CASE(bubbleSortReversedNeedsEverySwap)
{
    int a[] = {5, 4, 3, 2, 1};
    SortCounter c;
    bubbleSort(a, 5, &c);
    CHECK_EQ(c.swaps, 10LL);       // n(n-1)/2 inversions
    CHECK_EQ(c.comparisons, 10LL); // 4 + 3 + 2 + 1
}

TEST_CASE(selectionSortMakesAtMostNMinusOneSwaps)
{
    int a[] = {5, 4, 3, 2, 1, 0};
    SortCounter c;
    selectionSort(a, 6, &c);
    CHECK_EQ(c.comparisons, 15LL); // always n(n-1)/2
    CHECK(c.swaps <= 5);
}

TEST_CASE(insertionSortCountsShifts)
{
    int a[] = {3, 2, 1};
    SortCounter c;
    insertionSort(a, 3, &c);
    CHECK(isSorted(a, 3));
    CHECK_EQ(c.swaps, 3LL); // one inversion = one shift
}

TEST_CASE(mergeAndRadixCountMoves)
{
    int a[] = {4, 3, 2, 1};
    SortCounter m;
    mergeSort(a, 4, &m);
    CHECK(isSorted(a, 4));
    CHECK(m.comparisons > 0);
    CHECK_EQ(m.swaps, 8LL); // 2 + 2 + 4 copied back

    std::string w[] = {"CRANE", "APPLE", "BRAVE"};
    SortCounter r;
    radixSortWords(w, 3, 5, &r);
    CHECK(isSorted(w, 3));
    CHECK_EQ(r.comparisons, 0LL); // radix sort never compares
    CHECK_EQ(r.swaps, 15LL);      // 5 passes x 3 words
}

TEST_CASE(quickSortHandlesSortedInputWithoutQuadraticBlowUp)
{
    const int n = 2000;
    int* a = new int[n];
    for (int i = 0; i < n; i++)
        a[i] = i;
    SortCounter c;
    quickSort(a, n, &c);
    CHECK(isSorted(a, n));
    CHECK(c.comparisons < 60000LL); // n^2/2 would be ~2,000,000
    delete[] a;
}

TEST_CASE(quickSortIterativeKeepsStackShallow)
{
    const int n = 4096;
    int* a = new int[n];
    for (int i = 0; i < n; i++)
        a[i] = (i * 7919) % n; // a permutation of 0..n-1
    int depth = quickSortIterative(a, n);
    CHECK(isSorted(a, n));
    CHECK(depth <= 2 * 13 + 2); // O(log n): smaller part handled first
    delete[] a;
}

TEST_CASE(allSortsGiveTheSameResult)
{
    const int n = 300;
    int base[n];
    for (int i = 0; i < n; i++)
        base[i] = (i * 37 + 11) % 101;

    int b[n], s[n], ins[n], cm[n], m[n], q[n], qi[n];
    for (int i = 0; i < n; i++)
        b[i] = s[i] = ins[i] = cm[i] = m[i] = q[i] = qi[i] = base[i];

    bubbleSort(b, n);
    selectionSort(s, n);
    insertionSort(ins, n);
    combSort(cm, n);
    mergeSort(m, n);
    quickSort(q, n);
    quickSortIterative(qi, n);

    bool same = true;
    for (int i = 0; i < n; i++)
        same = same && b[i] == s[i] && s[i] == ins[i] && ins[i] == cm[i] && cm[i] == m[i] &&
               m[i] == q[i] && q[i] == qi[i];
    CHECK(same);
    CHECK(isSorted(b, n));
}

int main()
{
    return tf::runAll("Sort counters / bubble / selection / comb / quick");
}
