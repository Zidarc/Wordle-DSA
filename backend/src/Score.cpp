#include "Score.h"

using namespace std;

int Score::calculate(int maxAttempts, int attemptsUsed, int hintsUsed, Difficulty difficulty, bool won)
{
    return calculate(maxAttempts, attemptsUsed, hintsUsed, scoreMultiplier(difficulty), won);
}

// GUIDE 3.3: score formula
int Score::calculate(int maxAttempts, int attemptsUsed, int hintsUsed, double multiplier, bool won)
{
    if (!won || attemptsUsed <= 0 || attemptsUsed > maxAttempts)
        return 0;

    int base = (maxAttempts - attemptsUsed + 1) * 100;
    int penalty = hintsUsed * 50;
    int raw = base - penalty;
    if (raw < 0)
        raw = 0;

    int finalScore = (int)(raw * multiplier);
    return finalScore < 0 ? 0 : finalScore;
}
