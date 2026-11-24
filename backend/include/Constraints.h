#ifndef WORDLE_DSA_CONSTRAINTS_H
#define WORDLE_DSA_CONSTRAINTS_H

// Everything the feedback so far says about the secret word.
//
//   fixed_[pos]            letter known to be at pos (a green), or '\0'
//   banned_[pos][letter]   letter cannot be at pos (a yellow or gray there)
//   minCount_[letter]      the word holds at least this many of letter
//   maxCount_[letter]      ... and at most this many (known once a copy of
//                          the letter came back gray)
//
// Counting greens + yellows of a letter in one guess gives its minimum count.
// If the same guess also has that letter gray, the count is exact.
// Example: secret APPLE, guess PUPPY -> P: 1 yellow + 1 green + 1 gray,
// so APPLE has exactly two P's.
//
// The solver's backtracking uses allowsLetterAt() and fitsCounts() to prune
// a partial word as soon as it breaks a rule, and matches() for whole words.

#include "LetterState.h"

#include <string>

class Constraints
{
public:
    static constexpr int kLength = 5;
    static constexpr int kLetters = 26;
    static constexpr int kNoLimit = kLength;

    Constraints();

    // Adds one guess (5 letters A-Z, upper-case) and its feedback.
    // Throws std::invalid_argument for a malformed guess or null feedback.
    void add(const std::string& guess, const LetterState* feedback);
    void reset();

    // Can `letter` go at `pos`, looking at that one position only?
    bool allowsLetterAt(int pos, char letter) const;

    // Can a word that starts with `prefix` still meet the letter counts?
    // Too many of a letter fails at once; too few fails only if the
    // remaining positions are not enough to make up the difference.
    bool fitsCounts(const std::string& prefix) const;

    // Does a whole 5-letter word agree with every guess so far?
    bool matches(const std::string& word) const;

    char fixedAt(int pos) const { return fixed_[pos]; }
    int minCount(char letter) const { return minCount_[letter - 'A']; }
    int maxCount(char letter) const { return maxCount_[letter - 'A']; }
    int guessesAdded() const { return guesses_; }

private:
    char fixed_[kLength];
    bool banned_[kLength][kLetters];
    int minCount_[kLetters];
    int maxCount_[kLetters];
    int guesses_;
};

#endif // WORDLE_DSA_CONSTRAINTS_H
