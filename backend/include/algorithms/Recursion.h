#ifndef RECURSION_H
#define RECURSION_H

// Types of recursion (lab week 4), each shown on a small word task, and
// where the same type appears in the rest of the project.
//
//   Type       The recursive call ...                Also used in
//   ---------  ------------------------------------  -----------------------------
//   tail       is the very last thing done           binarySearch (Search.h)
//   head       comes first; the work happens on the  BST / AVL destroy (children
//              way back out                          first, then the node)
//   tree       is made more than once per call       mergeSort, quickSort,
//                                                    tree node counts
//   indirect   goes through another function         (A calls B, B calls A)
//   nested     takes a recursive call as argument    (Ackermann-style)
//
// Common issues: a missing or unreachable base case recurses until the call
// stack overflows; tree recursion can repeat work (naive Fibonacci); deep
// recursion on large input is replaced by an explicit stack (simulation of
// recursion) - see quickSortIterative and Solver::generateCandidatesIterative.

#include <string>

using namespace std;

// ----- tail recursion -----
// Counts how many times `letter` appears in word[from..]. The running total
// travels in `acc`, so nothing is left to do after the recursive call.
// GUIDE 2.5: one example per recursion type below
inline int countLetterTail(const string& word, char letter, int from = 0, int acc = 0)
{
    if (from >= (int)(word.size()))
        return acc;
    return countLetterTail(word, letter, from + 1, acc + (word[from] == letter ? 1 : 0));
}

// ----- head recursion -----
// Reverses a word: the recursive call happens first, and each letter is
// appended while the calls return.
inline string reverseHead(const string& word, int index = 0)
{
    if (index >= (int)(word.size()))
        return "";
    string rest = reverseHead(word, index + 1);
    return rest + word[index];
}

// ----- tree recursion -----
// Number of distinct letter orderings of `length` positions over an alphabet
// of size k, counted by branching once per letter: k^length calls at the
// bottom level. This is exactly the tree the solver's backtracking would
// walk without pruning (26^5).
inline long long countOrderings(int length, int k)
{
    if (length == 0)
        return 1;
    long long total = 0;
    for (int i = 0; i < k; ++i)
        total += countOrderings(length - 1, k);
    return total;
}

// Naive Fibonacci: tree recursion that repeats work. `calls` counts how many
// calls were made, to show the exponential blow-up.
inline long long fibonacciTree(int n, long long& calls)
{
    ++calls;
    if (n < 2)
        return n;
    return fibonacciTree(n - 1, calls) + fibonacciTree(n - 2, calls);
}

// ----- indirect (mutual) recursion -----
// A word is "alternating" if consonants and vowels alternate. alternatesFromVowel
// and alternatesFromConsonant call each other, one letter at a time.
inline bool isVowel(char c)
{
    return c == 'A' || c == 'E' || c == 'I' || c == 'O' || c == 'U';
}

bool alternatesFromConsonant(const string& word, int index);

inline bool alternatesFromVowel(const string& word, int index)
{
    if (index >= (int)(word.size()))
        return true;
    return isVowel(word[index]) && alternatesFromConsonant(word, index + 1);
}

inline bool alternatesFromConsonant(const string& word, int index)
{
    if (index >= (int)(word.size()))
        return true;
    return !isVowel(word[index]) && alternatesFromVowel(word, index + 1);
}

inline bool isAlternatingWord(const string& word)
{
    return alternatesFromVowel(word, 0) || alternatesFromConsonant(word, 0);
}

// ----- nested recursion -----
// McCarthy's 91 function: the argument of the outer call is itself a
// recursive call. Returns 91 for every n <= 100, and n - 10 above that.
inline int mcCarthy91(int n)
{
    if (n > 100)
        return n - 10;
    return mcCarthy91(mcCarthy91(n + 11));
}

#endif
