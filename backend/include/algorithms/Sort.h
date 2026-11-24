#ifndef SORT_H
#define SORT_H

#include <string>
#include <type_traits>

// Optional counters for the Sorting Lab. Every sort takes a SortCounter*
// that may be null; when it is not, each element comparison and each element
// move (a swap, or a copy in insertion / merge / radix sort) is counted.
struct SortCounter
{
    long long comparisons = 0;
    long long swaps = 0;
};

// a[i] < a[j], counted.
template <typename T>
bool sortLess(const T& x, const T& y, SortCounter* counter)
{
    if (counter)
        counter->comparisons++;
    return x < y;
}

template <typename Arr>
void sortSwap(Arr& a, int i, int j, SortCounter* counter)
{
    if (counter)
        counter->swaps++;
    typedef typename std::decay<decltype(a[0])>::type T;
    T temp = a[i];
    a[i] = a[j];
    a[j] = temp;
}

// ---------------------------------------------------------------------------
// Bubble sort: O(n^2) comparisons, O(n) on already sorted input because a
// pass with no swap ends the sort early.
// ---------------------------------------------------------------------------
template <typename Arr>
void bubbleSort(Arr& a, int n, SortCounter* counter = nullptr)
{
    for (int pass = 0; pass < n - 1; pass++)
    {
        bool swapped = false;
        for (int i = 0; i < n - 1 - pass; i++)
        {
            if (sortLess(a[i + 1], a[i], counter))
            {
                sortSwap(a, i, i + 1, counter);
                swapped = true;
            }
        }
        if (!swapped)
            break;
    }
}

// ---------------------------------------------------------------------------
// Selection sort: always n(n-1)/2 comparisons, but at most n-1 swaps, so it is
// the cheapest of the simple sorts when moving an element is expensive.
// ---------------------------------------------------------------------------
template <typename Arr>
void selectionSort(Arr& a, int n, SortCounter* counter = nullptr)
{
    for (int i = 0; i < n - 1; i++)
    {
        int smallest = i;
        for (int j = i + 1; j < n; j++)
        {
            if (sortLess(a[j], a[smallest], counter))
                smallest = j;
        }
        if (smallest != i)
            sortSwap(a, i, smallest, counter);
    }
}

template <typename Arr>
void insertionSort(Arr& a, int n)
{
    typedef typename std::decay<decltype(a[0])>::type T;
    for (int i = 1; i < n; i++)
    {
        T value = a[i];
        int j = i - 1;
        while (j >= 0 && value < a[j])
        {
            a[j + 1] = a[j];
            j--;
        }
        a[j + 1] = value;
    }
}

template <typename Arr, typename T>
void mergeRange(Arr& a, T* temp, int low, int mid, int high)
{
    int i = low;
    int j = mid + 1;
    int k = low;

    while (i <= mid && j <= high)
    {
        if (a[j] < a[i])
            temp[k++] = a[j++];
        else
            temp[k++] = a[i++];
    }
    while (i <= mid)
        temp[k++] = a[i++];
    while (j <= high)
        temp[k++] = a[j++];

    for (k = low; k <= high; k++)
        a[k] = temp[k];
}

template <typename Arr, typename T>
void mergeSortRange(Arr& a, T* temp, int low, int high)
{
    if (low >= high)
        return;

    int mid = low + (high - low) / 2;
    mergeSortRange(a, temp, low, mid);
    mergeSortRange(a, temp, mid + 1, high);
    mergeRange(a, temp, low, mid, high);
}

template <typename Arr>
void mergeSort(Arr& a, int n)
{
    if (n < 2)
        return;

    typedef typename std::decay<decltype(a[0])>::type T;
    T* temp = new T[n];
    mergeSortRange(a, temp, 0, n - 1);
    delete[] temp;
}

inline int sortLetterIndex(char c)
{
    if (c >= 'a' && c <= 'z')
        c = c - 'a' + 'A';
    if (c >= 'A' && c <= 'Z')
        return c - 'A';
    return 0;
}

template <typename Arr>
void radixSortWords(Arr& a, int n, int length = 5)
{
    if (n < 2)
        return;

    std::string* temp = new std::string[n];

    for (int pos = length - 1; pos >= 0; pos--)
    {
        int count[27] = {0};

        for (int i = 0; i < n; i++)
            count[sortLetterIndex(a[i][pos]) + 1]++;

        for (int c = 0; c < 26; c++)
            count[c + 1] += count[c];

        for (int i = 0; i < n; i++)
        {
            int d = sortLetterIndex(a[i][pos]);
            temp[count[d]] = a[i];
            count[d]++;
        }

        for (int i = 0; i < n; i++)
            a[i] = temp[i];
    }

    delete[] temp;
}

#endif