#include "FrequencyTable.h"

#include <stdexcept>

FrequencyTable::FrequencyTable()
    : counts_(nullptr), wordCount_(0)
{
    allocate();
}

FrequencyTable::FrequencyTable(const FrequencyTable& other)
    : counts_(nullptr), wordCount_(0)
{
    allocate();
    copyFrom(other);
}

FrequencyTable& FrequencyTable::operator=(const FrequencyTable& other)
{
    if (this != &other)
        copyFrom(other); // same fixed size, so the rows can be reused
    return *this;
}

FrequencyTable::~FrequencyTable()
{
    release();
}

void FrequencyTable::allocate()
{
    counts_ = new int*[kPositions];
    for (int p = 0; p < kPositions; ++p)
        counts_[p] = new int[kLetters];
    clear();
}

void FrequencyTable::release()
{
    if (counts_ == nullptr)
        return;
    for (int p = 0; p < kPositions; ++p)
        delete[] counts_[p];
    delete[] counts_;
    counts_ = nullptr;
}

void FrequencyTable::copyFrom(const FrequencyTable& other)
{
    for (int p = 0; p < kPositions; ++p)
        for (int l = 0; l < kLetters; ++l)
            counts_[p][l] = other.counts_[p][l];
    for (int l = 0; l < kLetters; ++l)
        totals_[l] = other.totals_[l];
    wordCount_ = other.wordCount_;
}

void FrequencyTable::clear()
{
    for (int p = 0; p < kPositions; ++p)
        for (int l = 0; l < kLetters; ++l)
            counts_[p][l] = 0;
    for (int l = 0; l < kLetters; ++l)
        totals_[l] = 0;
    wordCount_ = 0;
}

int FrequencyTable::letterIndex(char letter)
{
    if (letter >= 'a' && letter <= 'z')
        return letter - 'a';
    if (letter >= 'A' && letter <= 'Z')
        return letter - 'A';
    return -1;
}

void FrequencyTable::add(const std::string& word)
{
    if (static_cast<int>(word.size()) != kPositions)
        return;
    int idx[kPositions];
    for (int p = 0; p < kPositions; ++p)
    {
        idx[p] = letterIndex(word[p]);
        if (idx[p] < 0)
            return;
    }

    bool seen[kLetters] = {false};
    for (int p = 0; p < kPositions; ++p)
    {
        counts_[p][idx[p]]++;
        if (!seen[idx[p]])
        {
            seen[idx[p]] = true;
            totals_[idx[p]]++;
        }
    }
    ++wordCount_;
}

void FrequencyTable::build(const SafeArray<std::string>& words)
{
    clear();
    for (const std::string& w : words)
        add(w);
}

void FrequencyTable::build(const SinglyLinkedList<std::string>& words)
{
    clear();
    for (const std::string& w : words)
        add(w);
}

int FrequencyTable::countAt(int position, char letter) const
{
    int l = letterIndex(letter);
    if (position < 0 || position >= kPositions || l < 0)
        throw std::out_of_range("FrequencyTable::countAt: position or letter out of range");
    return counts_[position][l];
}

int FrequencyTable::wordsContaining(char letter) const
{
    int l = letterIndex(letter);
    if (l < 0)
        throw std::out_of_range("FrequencyTable::wordsContaining: not a letter");
    return totals_[l];
}

long long FrequencyTable::score(const std::string& word) const
{
    if (static_cast<int>(word.size()) != kPositions)
        return 0;
    long long total = 0;
    bool seen[kLetters] = {false};
    for (int p = 0; p < kPositions; ++p)
    {
        int l = letterIndex(word[p]);
        if (l < 0)
            return 0;
        total += counts_[p][l];
        if (!seen[l])
        {
            seen[l] = true;
            total += totals_[l];
        }
    }
    return total;
}
