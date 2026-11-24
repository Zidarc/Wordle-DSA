#ifndef WORDLE_DSA_SCORE_H
#define WORDLE_DSA_SCORE_H

// Wordle-DSA score calculation:
//   score = ((maxAttempts - attemptsUsed + 1) * 100 - hintsUsed * 50) * multiplier
//
// A lost game scores 0.
// Score never goes below 0.

#include "Difficulty.h"

class Score
{
public:
    static int calculate(int maxAttempts, int attemptsUsed, int hintsUsed, Difficulty difficulty, bool won);
    static int calculate(int maxAttempts, int attemptsUsed, int hintsUsed, double multiplier, bool won);
};

#endif // WORDLE_DSA_SCORE_H
