#ifndef WORDLE_DSA_MIN_HEAP_H
#define WORDLE_DSA_MIN_HEAP_H

// MinHeap<T>: a binary min-heap (priority queue) stored in a SafeArray.
//
// Used for: the top-K leaderboard (Day 8). Every saved score is streamed
// through a heap of size K = 10. The root is the smallest of the K best
// scores so far, so a new score only enters if it beats the root, and it
// replaces the root in O(log K). At the end the heap holds the best K scores.
//
// Layout: a complete binary tree stored level by level in an array.
//   parent(i) = (i - 1) / 2,  left(i) = 2i + 1,  right(i) = 2i + 2
// Heap property: every parent is <= its children, so the minimum is at index 0.
//
// Ordering: Compare(a, b) returns true when a should be closer to the root
// than b. The default, LessThan<T>, uses operator< (a min-heap). Passing a
// "greater than" comparator turns it into a max-heap.
//
// Operations on an empty heap (top, pop, replaceTop) throw an error.
//
// Rule of Three: the only resource is the SafeArray member, which already
// deep-copies itself, so the compiler-generated destructor, copy constructor
// and copy assignment are correct (they copy the SafeArray).
//
// Complexity (n = elements):
//   push / pop / replaceTop / pushBounded ..... O(log n)
//   top / size / empty ......................... O(1)
//   build from an array (heapify) .............. O(n)

#include "ds/SafeArray.h"

#include <cstddef>
#include <string>

using namespace std;

template <typename T>
struct LessThan
{
    bool operator()(const T& a, const T& b) const { return a < b; }
};

template <typename T, typename Compare = LessThan<T>>
class MinHeap
{
public:
    // ----- construction -----

    MinHeap() = default;

    // Builds a heap from existing values in O(n) with bottom-up heapify
    // (sift down every non-leaf node, last one first).
    explicit MinHeap(const SafeArray<T>& values)
        : data_(values)
    {
        for (size_t i = data_.size() / 2; i > 0; --i)
            siftDown(i - 1);
    }

    // ----- insertion / removal -----

    void push(const T& value)
    {
        data_.pushBack(value);
        siftUp(data_.size() - 1);
    }

    // Removes the root (the minimum).
    void pop()
    {
        checkNotEmpty();
        size_t last = data_.size() - 1;
        data_.swapAt(0, last);
        data_.popBack();
        if (!data_.empty())
            siftDown(0);
    }

    // Replaces the root with value and restores the heap property.
    // Cheaper than pop() followed by push(): one sift instead of two.
    void replaceTop(const T& value)
    {
        checkNotEmpty();
        data_[0] = value;
        siftDown(0);
    }

    // Top-K helper: keeps at most `capacity` elements, namely the `capacity`
    // "largest" ones seen so far (by Compare). Returns true if value was kept.
    //   - heap not full yet          -> push it
    //   - value beats the root       -> it replaces the root
    //   - otherwise                  -> discard it
    // GUIDE 1.10 / 3.6: top-10 leaderboard trick
    bool pushBounded(const T& value, size_t capacity)
    {
        if (capacity == 0)
            return false;
        if (data_.size() < capacity)
        {
            push(value);
            return true;
        }
        if (compare_(data_[0], value))
        {
            replaceTop(value);
            return true;
        }
        return false;
    }

    void clear() { data_.clear(); }

    // ----- access -----

    const T& top() const
    {
        checkNotEmpty();
        return data_[0];
    }

    size_t size() const { return data_.size(); }
    bool empty() const { return data_.empty(); }

    // The underlying array in heap order (not sorted). Used to hand the
    // top-K entries to merge sort, and by tests to check the heap property.
    const SafeArray<T>& items() const { return data_; }

    // Removes every element in priority order (smallest first) into a SafeArray.
    // The heap is empty afterwards. This is heap sort: O(n log n).
    SafeArray<T> drainSorted()
    {
        SafeArray<T> out;
        out.reserve(data_.size());
        while (!data_.empty())
        {
            out.pushBack(data_[0]);
            pop();
        }
        return out;
    }

    // True if every parent is ordered before (or equal to) its children.
    bool isValidHeap() const
    {
        for (size_t i = 1; i < data_.size(); ++i)
        {
            if (compare_(data_[i], data_[parent(i)]))
                return false;
        }
        return true;
    }

private:
    SafeArray<T> data_;
    Compare compare_;

    static size_t parent(size_t i) { return (i - 1) / 2; }
    static size_t left(size_t i) { return 2 * i + 1; }
    static size_t right(size_t i) { return 2 * i + 2; }

    // Moves the element at i up while it is ordered before its parent.
    void siftUp(size_t i)
    {
        while (i > 0 && compare_(data_[i], data_[parent(i)]))
        {
            data_.swapAt(i, parent(i));
            i = parent(i);
        }
    }

    // Moves the element at i down, swapping with its smaller child, until
    // neither child is ordered before it.
    // GUIDE 1.10: swap with the smaller child until the heap property holds
    void siftDown(size_t i)
    {
        const size_t n = data_.size();
        while (true)
        {
            size_t smallest = i;
            if (left(i) < n && compare_(data_[left(i)], data_[smallest]))
                smallest = left(i);
            if (right(i) < n && compare_(data_[right(i)], data_[smallest]))
                smallest = right(i);
            if (smallest == i)
                return;
            data_.swapAt(i, smallest);
            i = smallest;
        }
    }

    void checkNotEmpty() const
    {
        if (data_.empty())
            throw "MinHeap: the heap is empty";
    }
};

#endif // WORDLE_DSA_MIN_HEAP_H
