#include "WordPicker.h"

#include "algorithms/Shuffle.h"

#include <stdexcept>

WordPicker::WordPicker(const SafeArray<std::string>& words)
    : WordPicker(words, static_cast<unsigned int>(shuffleGenerator()()))
{
}

WordPicker::WordPicker(const SafeArray<std::string>& words, unsigned int seed)
    : words_(words), generator_(seed)
{
    startNewCycle();
}

// GUIDE 3.2: secret word = circular list cursor
std::string WordPicker::nextWord()
{
    if (rotation_.empty())
        throw std::logic_error("WordPicker::nextWord: no words to pick from");

    std::string word = rotation_.current();
    rotation_.advance();
    lastServed_ = word;
    ++servedThisCycle_;

    if (servedThisCycle_ == words_.size())
    {
        ++completedCycles_;
        startNewCycle();
    }
    return word;
}

// Shuffles the words and lays them out around the circular list, cursor on the first.
void WordPicker::startNewCycle()
{
    servedThisCycle_ = 0;
    rotation_.clear();

    const int n = static_cast<int>(words_.size());
    if (n == 0)
        return;

    fisherYatesWith(words_, n, generator_);

    // Do not let the first word of a new cycle repeat the last word of the old one.
    if (n > 1 && words_[0] == lastServed_)
        words_.swapAt(0, static_cast<std::size_t>(n - 1));

    for (const std::string& word : words_)
        rotation_.pushBack(word);
    rotation_.resetCursor();
}
