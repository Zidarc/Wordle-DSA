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

#include "Constraints.h"
#include "Dictionary.h"
#include "algorithms/ListSearch.h"
#include "ds/SinglyLinkedList.h"

#include <string>

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

private:
    const Dictionary& dictionary_;
    SinglyLinkedList<std::string> candidates_;
    Constraints constraints_;
};

#endif // WORDLE_DSA_SOLVER_H
