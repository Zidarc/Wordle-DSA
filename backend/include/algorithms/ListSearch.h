#ifndef LISTSEARCH_H
#define LISTSEARCH_H

// Searching a singly linked list (lab week 5: "search using arrays and
// linked lists"). Used on the solver's list of possible words.
//
// Linear search works exactly as on an array: O(n).
//
// Binary search needs the middle element, and a linked list has no index, so
// the middle is found with two pointers: `fast` moves two nodes for every one
// node `slow` moves, and when fast reaches the end, slow is in the middle.
//
//     start                         end
//       A -> B -> C -> D -> E -> F -> (null)
//       slow moves 1, fast moves 2 ...      slow stops at D
//
// Each round still only needs one comparison, so the number of comparisons is
// O(log n) like on an array. But finding each middle walks half of the
// remaining range, so the pointer moves add up to n/2 + n/4 + ... = O(n).
// ListSearchStats reports both numbers, which shows why binary search is only
// worthwhile on a structure with O(1) random access.
//
// Interpolation search is not offered: it needs to jump straight to a
// computed position, which a linked list cannot do.

#include "ds/SinglyLinkedList.h"

struct ListSearchStats
{
    int comparisons = 0;  // element comparisons
    int pointerMoves = 0; // node-to-node steps
};

// Returns the 0-based position of key, or -1.
template <typename T>
int linearSearchList(const SinglyLinkedList<T>& list, const T& key, ListSearchStats* stats = nullptr)
{
    int index = 0;
    for (typename SinglyLinkedList<T>::ConstIterator it = list.begin(); it != list.end(); ++it)
    {
        if (stats)
            stats->comparisons++;
        if (*it == key)
            return index;
        if (stats)
            stats->pointerMoves++;
        index++;
    }
    return -1;
}

// The list must be sorted. Returns the 0-based position of key, or -1.
// GUIDE 2.2: slow / fast pointers find each middle
template <typename T>
int binarySearchList(const SinglyLinkedList<T>& list, const T& key, ListSearchStats* stats = nullptr)
{
    typedef typename SinglyLinkedList<T>::ConstIterator It;

    It start = list.begin();
    It end = list.end();
    int startIndex = 0;

    while (start != end)
    {
        // Find the middle of [start, end) with slow / fast pointers.
        It slow = start;
        It fast = start;
        int offset = 0;
        while (fast != end)
        {
            ++fast;
            if (stats)
                stats->pointerMoves++;
            if (fast != end)
            {
                ++fast;
                ++slow;
                ++offset;
                if (stats)
                    stats->pointerMoves += 2;
            }
        }

        if (stats)
            stats->comparisons++;
        if (*slow == key)
            return startIndex + offset;

        if (key < *slow)
        {
            end = slow; // keep [start, slow)
        }
        else
        {
            start = slow; // keep (slow, end)
            ++start;
            startIndex += offset + 1;
        }
    }
    return -1;
}

#endif
