#include "Solver.h"

#include "algorithms/Sort.h"

using namespace std;

Solver::Solver(const Dictionary& dictionary)
    : dictionary_(dictionary)
{
    reset();
}

void Solver::reset()
{
    constraints_.reset();
    candidates_.clear();
    for (const string& word : dictionary_.sortedWords())
        candidates_.pushBack(word);
}

int Solver::applyGuess(const string& guess, const LetterState* feedback)
{
    constraints_.add(Dictionary::normalize(guess), feedback);

    const Constraints& rules = constraints_;
    size_t removed =
        candidates_.removeAllIf([&rules](const string& w) { return !rules.matches(w); });
    return (int)(removed);
}

int Solver::applyHint(int pos, char letter)
{
    if (letter >= 'a' && letter <= 'z')
        letter = (char)(letter - 'a' + 'A');
    constraints_.addKnownLetter(pos, letter);

    const Constraints& rules = constraints_;
    size_t removed =
        candidates_.removeAllIf([&rules](const string& w) { return !rules.matches(w); });
    return (int)(removed);
}

bool Solver::isCandidate(const string& word, ListSearchStats* stats) const
{
    return binarySearchList(candidates_, Dictionary::normalize(word), stats) != -1;
}

bool Solver::isDictionaryPrefix(const string& prefix, long long* comparisons) const
{
    const SafeArray<string>& words = dictionary_.sortedWords();
    int low = 0;
    int high = (int)(words.size()); // search in [low, high)
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
    if (low == (int)(words.size()))
        return false;
    return words[low].compare(0, prefix.size(), prefix) == 0;
}

SafeArray<string> Solver::generateCandidates(BacktrackStats* stats) const
{
    BacktrackStats local;
    SafeArray<string> out;
    string prefix;
    extend(prefix, out, local);
    local.solutions = (int)(out.size());
    if (stats)
        *stats = local;
    return out;
}

// One level of the recursion: choose the letter for position prefix.size().
// GUIDE 2.6: backtracking - choose, explore, un-choose
void Solver::extend(string& prefix, SafeArray<string>& out, BacktrackStats& stats) const
{
    const int depth = (int)(prefix.size());
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

namespace
{
struct Frame
{
    string prefix;
    char next = 'A'; // next letter to try at position prefix.size()
};
} // namespace

SafeArray<string> Solver::generateCandidatesIterative(BacktrackStats* stats) const
{
    BacktrackStats local;
    SafeArray<string> out;
    ArrayStack<Frame> stack;
    stack.push(Frame());

    while (!stack.empty())
    {
        if ((int)(stack.size()) > local.maxDepth)
            local.maxDepth = (int)(stack.size());

        Frame frame = stack.pop();
        if (frame.next > 'Z')
            continue; // every letter tried at this level: backtrack

        const int depth = (int)(frame.prefix.size());
        const char c = frame.next;

        Frame sameLevel = frame; // come back later for the following letter
        ++sameLevel.next;
        stack.push(sameLevel);

        ++local.nodesVisited;
        if (!constraints_.allowsLetterAt(depth, c))
        {
            ++local.pruned;
            continue;
        }

        string longer = frame.prefix + c;
        if (!constraints_.fitsCounts(longer) || !isDictionaryPrefix(longer, &local.prefixChecks))
        {
            ++local.pruned;
            continue;
        }

        if ((int)(longer.size()) == Constraints::kLength)
        {
            out.pushBack(longer);
        }
        else
        {
            Frame deeper;
            deeper.prefix = longer;
            stack.push(deeper);
        }
    }

    local.solutions = (int)(out.size());
    if (stats)
        *stats = local;
    return out;
}

// GUIDE 2.6: FrequencyTable score + quickSort ranking
SafeArray<RankedWord> Solver::suggestions(size_t count) const
{
    FrequencyTable table;
    table.build(candidates_);

    SafeArray<RankedWord> ranked;
    ranked.reserve(candidates_.size());
    for (const string& word : candidates_)
    {
        RankedWord r;
        r.word = word;
        r.score = table.score(word);
        ranked.pushBack(r);
    }
    quickSort(ranked, (int)(ranked.size()));

    while (ranked.size() > count)
        ranked.popBack();
    return ranked;
}
