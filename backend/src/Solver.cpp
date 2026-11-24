#include "Solver.h"

Solver::Solver(const Dictionary& dictionary)
    : dictionary_(dictionary)
{
    reset();
}

void Solver::reset()
{
    constraints_.reset();
    candidates_.clear();
    for (const std::string& word : dictionary_.sortedWords())
        candidates_.pushBack(word);
}

int Solver::applyGuess(const std::string& guess, const LetterState* feedback)
{
    constraints_.add(Dictionary::normalize(guess), feedback);

    const Constraints& rules = constraints_;
    std::size_t removed =
        candidates_.removeAllIf([&rules](const std::string& w) { return !rules.matches(w); });
    return static_cast<int>(removed);
}

bool Solver::isCandidate(const std::string& word, ListSearchStats* stats) const
{
    return binarySearchList(candidates_, Dictionary::normalize(word), stats) != -1;
}
