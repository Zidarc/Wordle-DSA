#ifndef WORDLE_DSA_LETTER_TRACKER_H
#define WORDLE_DSA_LETTER_TRACKER_H

// Tracks the colour of every letter A-Z for the on-screen keyboard.
//
//   status_[26]     direct addressing: status_[letter - 'A']
//   keyRows_        jagged array: 3 keyboard rows of different lengths
//                   QWERTYUIOP (10), ASDFGHJKL (9), ZXCVBNM (7)
//
// A letter's status only ever upgrades (unused -> absent -> present -> correct),
// so a letter that was green in one guess stays green on the keyboard.

#include "LetterState.h"

#include <string>

using namespace std;

class LetterTracker
{
public:
    static constexpr int kAlphabetSize = 26;
    static constexpr int kKeyboardRows = 3;

    LetterTracker();
    LetterTracker(const LetterTracker& other);
    LetterTracker& operator=(const LetterTracker& other);
    ~LetterTracker();

    // Applies one guess and its feedback (both of the same length).
    // Throws an error if feedback is null.
    void update(const string& guess, const LetterState* feedback);

    // Status of one letter, upper or lower case.
    // Throws an error for anything that is not A-Z.
    LetterState statusOf(char letter) const;

    // Sets every letter back to Unused (new game).
    void reset();

    // ----- jagged keyboard rows -----
    int keyboardRowCount() const { return kKeyboardRows; }
    int keyboardRowLength(int row) const;       // 10, 9, 7
    char keyAt(int row, int index) const;       // e.g. keyAt(0, 0) == 'Q'
    LetterState keyStatusAt(int row, int index) const;

private:
    LetterState status_[kAlphabetSize];
    char** keyRows_;
    int* rowLengths_;

    void buildKeyboard();
    void releaseKeyboard();
    void copyFrom(const LetterTracker& other);
    void checkKey(int row, int index) const;
    static int indexOf(char letter); // -1 for a non-letter
};

#endif // WORDLE_DSA_LETTER_TRACKER_H
