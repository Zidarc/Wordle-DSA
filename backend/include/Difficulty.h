#ifndef WORDLE_DSA_DIFFICULTY_H
#define WORDLE_DSA_DIFFICULTY_H

// Difficulty levels for Wordle-DSA.
//
// Easy:   8 attempts, 1.0x score multiplier, no extra rules
// Normal: 6 attempts, 1.5x score multiplier, no extra rules
// Hard:   5 attempts, 2.0x score multiplier, revealed hints must be reused in later guesses

#include <iostream>
#include <string>

enum class Difficulty
{
    Easy = 0,
    Normal = 1,
    Hard = 2
};

inline const char* toString(Difficulty d)
{
    switch (d)
    {
    case Difficulty::Easy:
        return "easy";
    case Difficulty::Hard:
        return "hard";
    case Difficulty::Normal:
    default:
        return "normal";
    }
}

inline Difficulty difficultyFromString(const std::string& str)
{
    std::string lower;
    for (char c : str)
        lower += static_cast<char>(c >= 'A' && c <= 'Z' ? c - 'A' + 'a' : c);

    if (lower == "easy")
        return Difficulty::Easy;
    if (lower == "hard")
        return Difficulty::Hard;
    return Difficulty::Normal;
}

inline int maxAttempts(Difficulty d)
{
    switch (d)
    {
    case Difficulty::Easy:
        return 8;
    case Difficulty::Hard:
        return 5;
    case Difficulty::Normal:
    default:
        return 6;
    }
}

inline double scoreMultiplier(Difficulty d)
{
    switch (d)
    {
    case Difficulty::Easy:
        return 1.0;
    case Difficulty::Hard:
        return 2.0;
    case Difficulty::Normal:
    default:
        return 1.5;
    }
}

inline bool requiresHintReuse(Difficulty d)
{
    return d == Difficulty::Hard;
}

inline std::ostream& operator<<(std::ostream& os, Difficulty d)
{
    return os << toString(d);
}

#endif // WORDLE_DSA_DIFFICULTY_H
