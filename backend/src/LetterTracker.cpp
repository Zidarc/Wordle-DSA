#include "LetterTracker.h"

using namespace std;


namespace
{
const char* const kKeyboardLayout[LetterTracker::kKeyboardRows] = {"QWERTYUIOP", "ASDFGHJKL",
                                                                    "ZXCVBNM"};
}

LetterTracker::LetterTracker() : keyRows_(nullptr), rowLengths_(nullptr)
{
    reset();
    buildKeyboard();
}

LetterTracker::LetterTracker(const LetterTracker& other) : keyRows_(nullptr), rowLengths_(nullptr)
{
    buildKeyboard();
    copyFrom(other);
}

LetterTracker& LetterTracker::operator=(const LetterTracker& other)
{
    if (this == &other)
        return *this;
    // The keyboard layout is fixed, so the rows already have the right sizes.
    copyFrom(other);
    return *this;
}

LetterTracker::~LetterTracker()
{
    releaseKeyboard();
}

// ---------------------------------------------------------------------------
// Letter status
// ---------------------------------------------------------------------------

// GUIDE 1.11: direct addressing status_[letter - 'A'], only upgrades
void LetterTracker::update(const string& guess, const LetterState* feedback)
{
    if (feedback == nullptr)
        throw "LetterTracker::update: feedback is null";

    for (size_t i = 0; i < guess.size(); ++i)
    {
        int index = indexOf(guess[i]);
        if (index < 0)
            continue;
        // Only upgrade, never downgrade.
        if ((int)(feedback[i]) > (int)(status_[index]))
            status_[index] = feedback[i];
    }
}

LetterState LetterTracker::statusOf(char letter) const
{
    int index = indexOf(letter);
    if (index < 0)
        throw "LetterTracker: not a letter";
    return status_[index];
}

void LetterTracker::reset()
{
    for (int i = 0; i < kAlphabetSize; ++i)
        status_[i] = LetterState::Unused;
}

int LetterTracker::indexOf(char letter)
{
    if (letter >= 'a' && letter <= 'z')
        return letter - 'a';
    if (letter >= 'A' && letter <= 'Z')
        return letter - 'A';
    return -1;
}

// ---------------------------------------------------------------------------
// Jagged keyboard rows
// ---------------------------------------------------------------------------

int LetterTracker::keyboardRowLength(int row) const
{
    checkKey(row, 0);
    return rowLengths_[row];
}

char LetterTracker::keyAt(int row, int index) const
{
    checkKey(row, index);
    return keyRows_[row][index];
}

LetterState LetterTracker::keyStatusAt(int row, int index) const
{
    return statusOf(keyAt(row, index));
}

void LetterTracker::buildKeyboard()
{
    keyRows_ = new char*[kKeyboardRows];
    rowLengths_ = new int[kKeyboardRows];
    for (int row = 0; row < kKeyboardRows; ++row)
    {
        int length = (int)(string(kKeyboardLayout[row]).size());
        rowLengths_[row] = length;
        keyRows_[row] = new char[length]; // each row has its own length
        for (int i = 0; i < length; ++i)
            keyRows_[row][i] = kKeyboardLayout[row][i];
    }
}

void LetterTracker::releaseKeyboard()
{
    if (keyRows_ != nullptr)
    {
        for (int row = 0; row < kKeyboardRows; ++row)
            delete[] keyRows_[row];
        delete[] keyRows_;
        keyRows_ = nullptr;
    }
    delete[] rowLengths_;
    rowLengths_ = nullptr;
}

void LetterTracker::copyFrom(const LetterTracker& other)
{
    for (int i = 0; i < kAlphabetSize; ++i)
        status_[i] = other.status_[i];
}

void LetterTracker::checkKey(int row, int index) const
{
    if (row < 0 || row >= kKeyboardRows || index < 0 || index >= rowLengths_[row])
        throw "LetterTracker: key is outside the keyboard";
}
