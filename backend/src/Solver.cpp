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

bool Solver::isDictionaryPrefix(const std::string& prefix, long long* comparisons) const
{
    const SafeArray<std::string>& words = dictionary_.sortedWords();
    int low = 0;
    int high = static_cast<int>(words.size()); // search in [low, high)
    while (low < high)
    {
        int mid = low + (high - low) / 2;
        if (comparisons)
            ++*comparisons;
        if (words[mid] < prefix)
            low = mid + 1;
        else
            high = mid;
    }
    if (low == static_cast<int>(words.size()))
        return false;
    return words[low].compare(0, prefix.size(), prefix) == 0;
}

SafeArray<std::string> Solver::generateCandidates(BacktrackStats* stats) const
{
    BacktrackStats local;
    SafeArray<std::string> out;
    std::string prefix;
    extend(prefix, out, local);
    local.solutions = static_cast<int>(out.size());
    if (stats)
        *stats = local;
    return out;
}

// One level of the recursion: choose the letter for position prefix.size().
void Solver::extend(std::string& prefix, SafeArray<std::string>& out, BacktrackStats& stats) const
{
    const int depth = static_cast<int>(prefix.size());
    if (depth > stats.maxDepth)
        stats.maxDepth = depth;

    if (depth == Constraints::kLength)
    {
        // Every prefix check passed, so prefix is a dictionary word that
        // follows every positional rule; the counts were checked on the way.
        out.pushBack(prefix);
        return;
    }

    for (char c = 'A'; c <= 'Z'; ++c)
    {
        ++stats.nodesVisited;
        if (!constraints_.allowsLetterAt(depth, c))
        {
            ++stats.pruned;
            continue;
        }

        prefix.push_back(c); // choose
        if (constraints_.fitsCounts(prefix) && isDictionaryPrefix(prefix, &stats.prefixChecks))
            extend(prefix, out, stats); // explore
        else
            ++stats.pruned;
        prefix.pop_back(); // un-choose (backtrack)
    }
}
