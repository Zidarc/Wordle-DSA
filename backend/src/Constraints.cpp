#include "Constraints.h"

using namespace std;


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
void Constraints::add(const string& guess, const LetterState* feedback)
{
    if (feedback == nullptr)
        throw "Constraints::add: feedback is null";
    if ((int)(guess.size()) != kLength)
        throw "Constraints::add: guess must have 5 letters";
    for (char c : guess)
    {
        if (c < 'A' || c > 'Z')
            throw "Constraints::add: guess must be upper-case A-Z";
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

// GUIDE 2.6: a hint is a green without a guess - fix it, then recount
void Constraints::addKnownLetter(int pos, char letter)
{
    if (pos < 0 || pos >= kLength)
        throw "Constraints::addKnownLetter: position out of range";
    if (letter < 'A' || letter > 'Z')
        throw "Constraints::addKnownLetter: letter must be upper-case A-Z";

    fixed_[pos] = letter;

    int fixedCopies = 0;
    for (int p = 0; p < kLength; ++p)
    {
        if (fixed_[p] == letter)
            ++fixedCopies;
    }
    int l = letter - 'A';
    if (fixedCopies > minCount_[l])
        minCount_[l] = fixedCopies;
}

bool Constraints::allowsLetterAt(int pos, char letter) const
{
    int l = letter - 'A';
    if (fixed_[pos] != '\0')
        return fixed_[pos] == letter;
    return !banned_[pos][l] && maxCount_[l] > 0;
}

bool Constraints::fitsCounts(const string& prefix) const
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
    int remaining = kLength - (int)(prefix.size());
    return stillNeeded <= remaining;
}

bool Constraints::matches(const string& word) const
{
    if ((int)(word.size()) != kLength)
        return false;
    for (int p = 0; p < kLength; ++p)
    {
        if (word[p] < 'A' || word[p] > 'Z' || !allowsLetterAt(p, word[p]))
            return false;
    }
    return fitsCounts(word);
}
