#ifndef WORDLE_DSA_SOLVER_H
#define WORDLE_DSA_SOLVER_H

// The solver: which dictionary words still fit all the feedback so far.
//
//   candidates_   singly linked list of the words still possible, in sorted
//                 order. After each guess, every word that breaks the new
//                 feedback is unlinked (one O(n) pass, no shifting as an
//                 array would need).
//   constraints_  what the feedback says (see Constraints.h)
//
// The candidate list is searched with linearSearchList / binarySearchList
// (ListSearch.h), so the DSA Insights panel can compare searching a linked
// list with searching the dictionary array.
//
// Backtracking (lab week 4): generateCandidates() finds the same words a
// second way, without looking at the list. It builds a word one letter at a
// time and abandons ("backtracks from") a partial word as soon as it cannot
// lead to an answer:
//
//                         (start)
//              /      /      |      \      \                 (diagram)
//            A      B  ...   C  ...  S  ...  Z     26 choices per position
//           ...    ...      / \    ...      x     x = pruned: the letter is
//                         CA   CR ...              gray, or banned here, or
//                         x    / \                 no dictionary word starts
//                            CRA  CRE              with this prefix
//                            ...
//
// A prefix is pruned when:
//   - the feedback rules the letter out at this position (gray / yellow /
//     another letter is green here),
//   - the prefix already uses a letter too often, or leaves too few places
//     for the letters the word must still contain,
//   - no dictionary word starts with the prefix (binary search on the sorted
//     dictionary array).
// Without pruning there would be 26^5 = 11,881,376 leaves to try.

#include "Constraints.h"
#include "Dictionary.h"
#include "algorithms/ListSearch.h"
#include "ds/SafeArray.h"
#include "ds/SinglyLinkedList.h"

#include <string>

// Counters for one backtracking run, for the DSA Insights panel.
struct BacktrackStats
{
    long long nodesVisited = 0; // letters tried
    long long pruned = 0;       // branches abandoned
    long long prefixChecks = 0; // binary-search comparisons for "is this a prefix?"
    int maxDepth = 0;           // deepest recursion level / largest stack size
    int solutions = 0;
};

class Solver
{
public:
    // dictionary must outlive the solver.
    explicit Solver(const Dictionary& dictionary);

    // Applies one guess and its feedback; returns how many words were removed.
    int applyGuess(const std::string& guess, const LetterState* feedback);

    // Back to the full dictionary, no constraints.
    void reset();

    std::size_t remaining() const { return candidates_.size(); }
    const SinglyLinkedList<std::string>& candidates() const { return candidates_; }
    const Constraints& constraints() const { return constraints_; }

    // Is the word still possible? Binary search on the sorted linked list.
    bool isCandidate(const std::string& word, ListSearchStats* stats = nullptr) const;

    // Backtracking search for every dictionary word that fits the
    // constraints, recursively. The result is in sorted order and always
    // equals candidates().
    SafeArray<std::string> generateCandidates(BacktrackStats* stats = nullptr) const;

    // True if some dictionary word starts with prefix. Binary search for the
    // first word >= prefix, then compare its first letters.
    bool isDictionaryPrefix(const std::string& prefix, long long* comparisons = nullptr) const;

private:
    const Dictionary& dictionary_;

    void extend(std::string& prefix, SafeArray<std::string>& out, BacktrackStats& stats) const;
    SinglyLinkedList<std::string> candidates_;
    Constraints constraints_;
};

#endif // WORDLE_DSA_SOLVER_H
