#include "Constraints.h"

#include <stdexcept>

Constraints::Constraints()
{
    reset();
}

void Constraints::reset()
{
    for (int p = 0; p < kLength; ++p)
    {
        fixed_[p] = '\0';
        for (int l = 0; l < kLetters; ++l)
            banned_[p][l] = false;
    }
    for (int l = 0; l < kLetters; ++l)
    {
        minCount_[l] = 0;
        maxCount_[l] = kNoLimit;
    }
    guesses_ = 0;
}

// GUIDE 2.6: greens fix a position, grays cap a letter count
void Constraints::add(const std::string& guess, const LetterState* feedback)
{
    if (feedback == nullptr)
        throw std::invalid_argument("Constraints::add: feedback is null");
    if (static_cast<int>(guess.size()) != kLength)
        throw std::invalid_argument("Constraints::add: guess must have 5 letters");
    for (char c : guess)
    {
        if (c < 'A' || c > 'Z')
            throw std::invalid_argument("Constraints::add: guess must be upper-case A-Z");
    }

    int marked[kLetters] = {0}; // greens + yellows of each letter in this guess
    bool gray[kLetters] = {false};

    for (int p = 0; p < kLength; ++p)
    {
        int l = guess[p] - 'A';
        switch (feedback[p])
        {
        case LetterState::Correct:
            fixed_[p] = guess[p];
            ++marked[l];
            break;
        case LetterState::Present:
            banned_[p][l] = true;
            ++marked[l];
            break;
        default: // Absent
            banned_[p][l] = true;
            gray[l] = true;
            break;
        }
    }

    for (int l = 0; l < kLetters; ++l)
    {
        if (marked[l] > minCount_[l])
            minCount_[l] = marked[l];
        if (gray[l] && marked[l] < maxCount_[l])
            maxCount_[l] = marked[l];
    }
    ++guesses_;
}

bool Constraints::allowsLetterAt(int pos, char letter) const
{
    int l = letter - 'A';
    if (fixed_[pos] != '\0')
        return fixed_[pos] == letter;
    return !banned_[pos][l] && maxCount_[l] > 0;
}

bool Constraints::fitsCounts(const std::string& prefix) const
{
    int have[kLetters] = {0};
    for (char c : prefix)
        ++have[c - 'A'];

    int stillNeeded = 0;
    for (int l = 0; l < kLetters; ++l)
    {
        if (have[l] > maxCount_[l])
            return false;
        if (have[l] < minCount_[l])
            stillNeeded += minCount_[l] - have[l];
    }
    int remaining = kLength - static_cast<int>(prefix.size());
    return stillNeeded <= remaining;
}

bool Constraints::matches(const std::string& word) const
{
    if (static_cast<int>(word.size()) != kLength)
        return false;
    for (int p = 0; p < kLength; ++p)
    {
        if (word[p] < 'A' || word[p] > 'Z' || !allowsLetterAt(p, word[p]))
            return false;
    }
    return fitsCounts(word);
}
