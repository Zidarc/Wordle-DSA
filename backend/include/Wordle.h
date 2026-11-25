#ifndef WORDLE_DSA_WORDLE_H
#define WORDLE_DSA_WORDLE_H

// The secret word and the rule that colours a guess.
//
// The secret word's distinct letters are stored in a BST, so
// "is this letter anywhere in the word?" is an O(log n) lookup.
//
// evaluate() colours a guess in two passes so repeated letters are right:
//   1. Greens: every letter in its correct position. Each one uses up one
//      count of that letter from the secret word.
//   2. Yellows: for each remaining letter, if the BST says the letter is in
//      the word and an unused count is left, it is yellow; otherwise gray.
//
// Example, secret APPLE:  PUPPY -> yellow gray green gray gray

#include "LetterState.h"
#include "ds/BST.h"

#include <string>

using namespace std;

struct GuessFeedback
{
    static constexpr int kLength = 5;

    string guess; // upper-case
    LetterState states[kLength] = {LetterState::Absent, LetterState::Absent, LetterState::Absent,
                                   LetterState::Absent, LetterState::Absent};
    int bstComparisons = 0; // comparisons made in the secret-letter BST, for DSA Insights

    bool isWin() const;
};

class Wordle
{
public:
    // Throws an error unless the secret is 5 letters A-Z (any case).
    explicit Wordle(const string& secret);

    // Colours a guess. Throws an error unless it is 5 letters A-Z.
    // Does not check the dictionary; that is GameSession's job.
    GuessFeedback evaluate(const string& guess) const;

    // True if the letter appears anywhere in the secret word (BST lookup).
    bool containsLetter(char letter) const;
    bool containsLetter(char letter, int& comparisons) const;

    const string& secret() const { return secret_; }
    int distinctLetters() const { return (int)(letters_.size()); }
    int letterTreeHeight() const { return letters_.height(); }

private:
    string secret_;
    BST<char> letters_;

    static char toUpper(char c);
};

#endif // WORDLE_DSA_WORDLE_H
