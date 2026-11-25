#include "TestFramework.h"
#include "ds/MinHeap.h"
#include "ds/SafeArray.h"

#include <cstddef>
#include <string>

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

template <typename T>
bool isAscending(const SafeArray<T>& arr)
{
    for (std::size_t i = 1; i < arr.size(); ++i)
    {
        if (arr[i] < arr[i - 1])
            return false;
    }
    return true;
}

// A leaderboard-like entry: ordered by score only.
struct ScoreEntry
{
    std::string name;
    int score;

    bool operator<(const ScoreEntry& other) const { return score < other.score; }
};

template <typename T>
struct GreaterThan
{
    bool operator()(const T& a, const T& b) const { return a > b; }
};

// ===========================================================================
// Empty heap
// ===========================================================================

TEST_CASE(minHeapDefaultConstructedIsEmpty)
{
    MinHeap<int> heap;
    CHECK(heap.empty());
    CHECK_EQ(heap.size(), 0u);
    CHECK(heap.isValidHeap());
}

TEST_CASE(minHeapEmptyOperationsThrow)
{
    MinHeap<int> heap;
    CHECK_THROWS(heap.top());
    CHECK_THROWS(heap.pop());
    CHECK_THROWS(heap.replaceTop(1));
}

// ===========================================================================
// Insert / extract-min order
// ===========================================================================

TEST_CASE(minHeapSingleElement)
{
    MinHeap<int> heap;
    heap.push(42);
    CHECK_EQ(heap.top(), 42);
    CHECK_EQ(heap.size(), 1u);
    heap.pop();
    CHECK(heap.empty());
}

TEST_CASE(minHeapTopIsAlwaysTheMinimum)
{
    MinHeap<int> heap;
    heap.push(50);
    CHECK_EQ(heap.top(), 50);
    heap.push(20);
    CHECK_EQ(heap.top(), 20);
    heap.push(80);
    CHECK_EQ(heap.top(), 20);
    heap.push(10);
    CHECK_EQ(heap.top(), 10);
    CHECK(heap.isValidHeap());
}

TEST_CASE(minHeapExtractsInAscendingOrder)
{
    MinHeap<int> heap;
    const int values[] = {35, 7, 92, 7, 18, 64, 1, 50, 23, 88, 0, 41};
    for (int v : values)
    {
        heap.push(v);
        CHECK(heap.isValidHeap());
    }

    int previous = -1;
    bool ascending = true;
    std::size_t popped = 0;
    while (!heap.empty())
    {
        if (heap.top() < previous)
            ascending = false;
        previous = heap.top();
        heap.pop();
        ++popped;
        if (!heap.isValidHeap())
            ascending = false;
    }
    CHECK(ascending);
    CHECK_EQ(popped, 12u);
}

TEST_CASE(minHeapHandlesDuplicates)
{
    MinHeap<int> heap;
    for (int i = 0; i < 5; ++i)
        heap.push(3);
    heap.push(1);
    CHECK_EQ(heap.top(), 1);
    heap.pop();
    for (int i = 0; i < 5; ++i)
    {
        CHECK_EQ(heap.top(), 3);
        heap.pop();
    }
    CHECK(heap.empty());
}

TEST_CASE(minHeapDrainSortedIsHeapSort)
{
    MinHeap<int> heap;
    for (int i = 0; i < 500; ++i)
        heap.push((i * 7919) % 1000); // scrambled order
    SafeArray<int> sorted = heap.drainSorted();
    CHECK_EQ(sorted.size(), 500u);
    CHECK(isAscending(sorted));
    CHECK(heap.empty());
}

TEST_CASE(minHeapReplaceTop)
{
    MinHeap<int> heap;
    heap.push(10);
    heap.push(20);
    heap.push(30);
    heap.replaceTop(25); // 10 out, 25 in
    CHECK_EQ(heap.size(), 3u);
    CHECK_EQ(heap.top(), 20);
    CHECK(heap.isValidHeap());
}

// ===========================================================================
// Build from an array (heapify)
// ===========================================================================

TEST_CASE(minHeapHeapifyFromArray)
{
    SafeArray<int> values;
    for (int v = 100; v > 0; --v) // descending: worst case for a min-heap
        values.pushBack(v);

    MinHeap<int> heap(values);
    CHECK_EQ(heap.size(), 100u);
    CHECK(heap.isValidHeap());
    CHECK_EQ(heap.top(), 1);
    CHECK(isAscending(heap.drainSorted()));
}

TEST_CASE(minHeapHeapifyEmptyAndSingle)
{
    SafeArray<int> none;
    MinHeap<int> empty(none);
    CHECK(empty.empty());

    SafeArray<int> one;
    one.pushBack(7);
    MinHeap<int> single(one);
    CHECK_EQ(single.top(), 7);
}

// ===========================================================================
// Top-K (leaderboard use)
// ===========================================================================

TEST_CASE(minHeapTopKOfALargerList)
{
    MinHeap<int> heap;
    const std::size_t k = 10;
    for (int i = 1; i <= 1000; ++i)
        heap.pushBounded((i * 37) % 1000 + 1, k); // 1..1000, scrambled

    CHECK_EQ(heap.size(), k);
    CHECK_EQ(heap.top(), 991); // smallest of the best 10

    SafeArray<int> best = heap.drainSorted();
    bool exact = true;
    for (std::size_t i = 0; i < k; ++i)
    {
        if (best[i] != static_cast<int>(991 + i))
            exact = false;
    }
    CHECK(exact);
}

TEST_CASE(minHeapTopKWithFewerItemsThanK)
{
    MinHeap<int> heap;
    CHECK(heap.pushBounded(5, 10));
    CHECK(heap.pushBounded(3, 10));
    CHECK_EQ(heap.size(), 2u);
    CHECK(!heap.pushBounded(1, 0)); // capacity 0 keeps nothing
}

TEST_CASE(minHeapTopKRejectsScoresThatDoNotBeatTheRoot)
{
    MinHeap<int> heap;
    heap.pushBounded(50, 3);
    heap.pushBounded(60, 3);
    heap.pushBounded(70, 3);
    CHECK(!heap.pushBounded(40, 3));  // worse than everything kept
    CHECK(!heap.pushBounded(50, 3));  // equal to the root: not better
    CHECK(heap.pushBounded(65, 3));   // beats the root (50)
    CHECK_EQ(heap.top(), 60);
}

TEST_CASE(minHeapWithStructEntries)
{
    MinHeap<ScoreEntry> heap;
    heap.pushBounded(ScoreEntry{"Ali", 525}, 2);
    heap.pushBounded(ScoreEntry{"Abbas", 300}, 2);
    heap.pushBounded(ScoreEntry{"Hussain", 800}, 2);

    CHECK_EQ(heap.size(), 2u);
    CHECK_EQ(heap.top().name, std::string("Ali"));
    heap.pop();
    CHECK_EQ(heap.top().name, std::string("Hussain"));
}

TEST_CASE(maxHeapWithCustomComparator)
{
    MinHeap<int, GreaterThan<int>> heap;
    heap.push(5);
    heap.push(50);
    heap.push(20);
    CHECK_EQ(heap.top(), 50);
    CHECK(heap.isValidHeap());
}

// ===========================================================================
// Copying
// ===========================================================================

TEST_CASE(minHeapCopyIsIndependent)
{
    MinHeap<int> original;
    original.push(5);
    original.push(1);
    original.push(9);

    MinHeap<int> copy(original);
    copy.pop();
    copy.push(0);

    CHECK_EQ(original.size(), 3u);
    CHECK_EQ(original.top(), 1);
    CHECK_EQ(copy.top(), 0);

    MinHeap<int> assigned;
    assigned.push(100);
    assigned = original;
    assigned.clear();
    CHECK_EQ(original.size(), 3u);
}

int main()
{
    return tf::runAll("MinHeap");
}
