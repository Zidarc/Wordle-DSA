#ifndef STRINGSEARCH_H
#define STRINGSEARCH_H

#include <string>

using namespace std;

// KMP string search. Used to search the leaderboard by part of a player's name.
// An empty pattern matches at index 0 for kmpSearch and counts as 0 matches.

inline char kmpChar(char c, bool ignoreCase)
{
    if (ignoreCase && c >= 'A' && c <= 'Z')
        return c - 'A' + 'a';
    return c;
}

// lps[i] = length of the longest proper prefix of pattern[0..i]
// that is also a suffix of it. lps must have room for pattern.length() ints.
// GUIDE 2.7: LPS table = where to resume after a mismatch
inline void buildLps(const string& pattern, int* lps, bool ignoreCase = false)
{
    int m = (int)pattern.length();
    if (m == 0)
        return;

    lps[0] = 0;
    int len = 0;
    int i = 1;
    while (i < m)
    {
        if (kmpChar(pattern[i], ignoreCase) == kmpChar(pattern[len], ignoreCase))
        {
            len++;
            lps[i] = len;
            i++;
        }
        else if (len > 0)
        {
            len = lps[len - 1];
        }
        else
        {
            lps[i] = 0;
            i++;
        }
    }
}

// Returns the index of the first match, or -1. comparisons is optional.
inline int kmpSearch(const string& text, const string& pattern,
                     int* comparisons = nullptr, bool ignoreCase = false)
{
    int n = (int)text.length();
    int m = (int)pattern.length();
    if (m == 0)
        return 0;
    if (m > n)
        return -1;

    int* lps = new int[m];
    buildLps(pattern, lps, ignoreCase);

    int result = -1;
    int i = 0;
    int j = 0;
    while (i < n)
    {
        if (comparisons)
            (*comparisons)++;

        if (kmpChar(text[i], ignoreCase) == kmpChar(pattern[j], ignoreCase))
        {
            i++;
            j++;
            if (j == m)
            {
                result = i - j;
                break;
            }
        }
        else if (j > 0)
        {
            j = lps[j - 1];
        }
        else
        {
            i++;
        }
    }

    delete[] lps;
    return result;
}

inline bool kmpContains(const string& text, const string& pattern, bool ignoreCase = false)
{
    return kmpSearch(text, pattern, nullptr, ignoreCase) != -1;
}

// Counts all matches, including overlapping ones ("AA" in "AAAAA" is 4).
inline int kmpCountMatches(const string& text, const string& pattern, bool ignoreCase = false)
{
    int n = (int)text.length();
    int m = (int)pattern.length();
    if (m == 0 || m > n)
        return 0;

    int* lps = new int[m];
    buildLps(pattern, lps, ignoreCase);

    int count = 0;
    int i = 0;
    int j = 0;
    while (i < n)
    {
        if (kmpChar(text[i], ignoreCase) == kmpChar(pattern[j], ignoreCase))
        {
            i++;
            j++;
            if (j == m)
            {
                count++;
                j = lps[j - 1];
            }
        }
        else if (j > 0)
        {
            j = lps[j - 1];
        }
        else
        {
            i++;
        }
    }

    delete[] lps;
    return count;
}

#endif