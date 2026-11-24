#ifndef WORDLE_DSA_SORTING_LAB_H
#define WORDLE_DSA_SORTING_LAB_H

// Sorting Lab: sorts the same shuffled copy of the dictionary with seven
// algorithms and reports each one's comparisons, element moves and time.
//
//   bubble, selection, insertion, comb   elementary, O(n^2)  (lab week 5)
//   merge, quick, radix                  advanced            (lab week 7)
//
// Every algorithm gets an identical input: the dictionary is copied into a
// SafeArray and shuffled with Fisher-Yates using a fixed seed, then each run
// sorts its own copy of that array. Afterwards the result is checked against
// the dictionary's own (radix-sorted) order.
//
// Served by GET /api/dsa/sort.

#include "ds/SafeArray.h"

#include <cstddef>
#include <string>

struct SortRun
{
    std::string algorithm;     // "bubble", "selection", ...
    std::string complexity;    // average-case Big-O, for display
    long long comparisons = 0;
    long long swaps = 0;       // swaps or element moves
    long long microseconds = 0;
    bool sortedCorrectly = false;
};

class SortingLab
{
public:
    static constexpr int kAlgorithmCount = 7;

    // words: the dictionary in sorted order (Dictionary::sortedWords()).
    // limit: sort only the first `limit` words of the shuffled copy (0 = all),
    // so the O(n^2) sorts stay quick on a large word list.
    SortingLab(const SafeArray<std::string>& words, unsigned int seed = 2513, std::size_t limit = 0);

    // Runs all seven algorithms, in the order listed above.
    SafeArray<SortRun> runAll() const;

    // Runs one algorithm by name. Throws std::invalid_argument for an unknown name.
    SortRun run(const std::string& algorithm) const;

    std::size_t inputSize() const { return input_.size(); }
    const SafeArray<std::string>& input() const { return input_; }

    // The names accepted by run(), in display order.
    static const char* algorithmName(int index);

private:
    SafeArray<std::string> input_;    // shuffled
    SafeArray<std::string> expected_; // input_ in sorted order
};

#endif // WORDLE_DSA_SORTING_LAB_H
