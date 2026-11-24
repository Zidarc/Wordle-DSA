#ifndef SEARCH_H
#define SEARCH_H

#include <string>

// GUIDE 2.1: word -> base-26 number, so interpolation search has numeric keys
inline long wordToKey(const std::string& word)
{
    long key = 0;
    for (int i = 0; i < (int)word.length(); i++)
    {
        char c = word[i];
        if (c >= 'a' && c <= 'z')
            c = c - 'a' + 'A';
        int value = 0;
        if (c >= 'A' && c <= 'Z')
            value = c - 'A';
        key = key * 26 + value;
    }
    return key;
}

// GUIDE 2.1: O(n) - the baseline in DSA Insights
template <typename Arr, typename T>
int linearSearch(const Arr& a, int n, const T& key, int* comparisons = nullptr)
{
    for (int i = 0; i < n; i++)
    {
        if (comparisons)
            (*comparisons)++;
        if (a[i] == key)
            return i;
    }
    return -1;
}

// The array must be sorted.
// GUIDE 2.1 / 2.5: O(log n), tail recursion
template <typename Arr, typename T>
int binarySearch(const Arr& a, int low, int high, const T& key, int* comparisons = nullptr)
{
    if (low > high)
        return -1;

    int mid = low + (high - low) / 2;
    if (comparisons)
        (*comparisons)++;

    if (a[mid] == key)
        return mid;
    if (key < a[mid])
        return binarySearch(a, low, mid - 1, key, comparisons);
    return binarySearch(a, mid + 1, high, key, comparisons);
}

// The array of words must be sorted, all words the same length and case.
// GUIDE 2.1: estimates the position instead of taking the middle
template <typename Arr>
int interpolationSearch(const Arr& a, int n, const std::string& key, int* comparisons = nullptr)
{
    int low = 0;
    int high = n - 1;
    long keyValue = wordToKey(key);

    while (low <= high)
    {
        long lowValue = wordToKey(a[low]);
        long highValue = wordToKey(a[high]);

        if (keyValue < lowValue || keyValue > highValue)
            return -1;

        int pos = low;
        if (highValue != lowValue)
            pos = low + (int)(((long long)(keyValue - lowValue) * (high - low)) / (highValue - lowValue));

        if (comparisons)
            (*comparisons)++;

        if (a[pos] == key)
            return pos;
        if (a[pos] < key)
            low = pos + 1;
        else
            high = pos - 1;
    }
    return -1;
}

#endif