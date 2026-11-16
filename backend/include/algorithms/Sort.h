#ifndef SORT_H
#define SORT_H

#include <string>
#include <type_traits>

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