#include "WordPicker.h"

#include "algorithms/Shuffle.h"

using namespace std;


WordPicker::WordPicker(const SafeArray<string>& words)
    : WordPicker(words, (unsigned int)(shuffleGenerator()()))
{
}

WordPicker::WordPicker(const SafeArray<string>& words, unsigned int seed)
    : words_(words), generator_(seed)
{
    startNewCycle();
}

// GUIDE 3.2: secret word = circular list cursor
string WordPicker::nextWord()
{
    if (rotation_.empty())
        throw "WordPicker::nextWord: no words to pick from";

    string word = rotation_.current();
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

    const int n = (int)(words_.size());
    if (n == 0)
        return;

    fisherYatesWith(words_, n, generator_);

    // Do not let the first word of a new cycle repeat the last word of the old one.
    if (n > 1 && words_[0] == lastServed_)
        words_.swapAt(0, (size_t)(n - 1));

    for (const string& word : words_)
        rotation_.pushBack(word);
    rotation_.resetCursor();
}
