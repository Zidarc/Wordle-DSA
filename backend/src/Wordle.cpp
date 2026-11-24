#include "Wordle.h"

#include "Dictionary.h"
#include "algorithms/Sort.h"

#include <stdexcept>

bool GuessFeedback::isWin() const
{
    for (int i = 0; i < kLength; ++i)
    {
        if (states[i] != LetterState::Correct)
            return false;
    }
    return true;
}

Wordle::Wordle(const std::string& secret) : secret_(Dictionary::normalize(secret))
{
    if (!Dictionary::isWellFormed(secret_))
        throw std::invalid_argument("Wordle: the secret word must be 5 letters A-Z");

    // Sort the letters, then insert the median first and each half's middle
    // next, so the tree is balanced (height 3). CRANE -> A C E N R gives
    //
    //          E
    //        /   \
    //       C     R
    //      /     /
    //     A     N
    //
    // Repeated letters are ignored by the BST.
    char sorted[GuessFeedback::kLength];
    for (int i = 0; i < GuessFeedback::kLength; ++i)
        sorted[i] = secret_[i];
    insertionSort(sorted, GuessFeedback::kLength);

    const int order[GuessFeedback::kLength] = {2, 1, 4, 0, 3};
    for (int i = 0; i < GuessFeedback::kLength; ++i)
        letters_.insert(sorted[order[i]]);
}

// GUIDE 3.3: two-pass colouring (greens first, then yellows / grays)
GuessFeedback Wordle::evaluate(const std::string& guess) const
{
    GuessFeedback result;
    result.guess = Dictionary::normalize(guess);
    if (!Dictionary::isWellFormed(result.guess))
        throw std::invalid_argument("Wordle::evaluate: a guess must be 5 letters A-Z");

    // Unused count of each letter in the secret word, after the greens.
    int remaining[26] = {0};
    for (int i = 0; i < GuessFeedback::kLength; ++i)
        ++remaining[secret_[i] - 'A'];

    // Pass 1: greens.
    for (int i = 0; i < GuessFeedback::kLength; ++i)
    {
        if (result.guess[i] == secret_[i])
        {
            result.states[i] = LetterState::Correct;
            --remaining[secret_[i] - 'A'];
        }
    }

    // Pass 2: yellows and grays.
    for (int i = 0; i < GuessFeedback::kLength; ++i)
    {
        if (result.states[i] == LetterState::Correct)
            continue;

        char letter = result.guess[i];
        if (containsLetter(letter, result.bstComparisons) && remaining[letter - 'A'] > 0)
        {
            result.states[i] = LetterState::Present;
            --remaining[letter - 'A'];
        }
        else
        {
            result.states[i] = LetterState::Absent;
        }
    }
    return result;
}

bool Wordle::containsLetter(char letter) const
{
    return letters_.contains(toUpper(letter));
}

bool Wordle::containsLetter(char letter, int& comparisons) const
{
    return letters_.contains(toUpper(letter), comparisons);
}

char Wordle::toUpper(char c)
{
    if (c >= 'a' && c <= 'z')
        return static_cast<char>(c - 'a' + 'A');
    return c;
}
