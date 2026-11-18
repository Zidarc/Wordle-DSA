#ifndef WORDLE_DSA_LETTER_STATE_H
#define WORDLE_DSA_LETTER_STATE_H

// Colour of one letter. The order matters: a letter's status on the keyboard
// may only move up this list (Unused -> Absent -> Present -> Correct).
enum class LetterState
{
    Unused = 0,  // not guessed yet
    Absent = 1,  // gray
    Present = 2, // yellow
    Correct = 3  // green
};

// Lower-case name used in the JSON sent to the frontend.
inline const char* toString(LetterState state)
{
    switch (state)
    {
    case LetterState::Absent:
        return "absent";
    case LetterState::Present:
        return "present";
    case LetterState::Correct:
        return "correct";
    case LetterState::Unused:
    default:
        return "unused";
    }
}

#endif // WORDLE_DSA_LETTER_STATE_H
