#ifndef WORDLE_DSA_FREQUENCY_TABLE_H
#define WORDLE_DSA_FREQUENCY_TABLE_H

// Letter frequencies of a word list, as a dynamic 2D array (lab week 2):
//
//   counts_[position][letter]   how many words have `letter` at `position`
//                               5 rows (positions) x 26 columns (A-Z)
//   totals_[letter]             how many words contain `letter` at least once
//
// The solver uses it to rank possible words: a word made of common letters in
// common positions splits the remaining words best, so it is a good guess.
//
// counts_ is allocated with new[] as an array of row pointers, so the class
// follows the Rule of Three.

#include "ds/SafeArray.h"
#include "ds/SinglyLinkedList.h"

#include <string>

using namespace std;

class FrequencyTable
{
public:
    static constexpr int kPositions = 5;
    static constexpr int kLetters = 26;

    FrequencyTable();
    FrequencyTable(const FrequencyTable& other);
    FrequencyTable& operator=(const FrequencyTable& other);
    ~FrequencyTable();

    // Clears the table, then counts every word (upper-case, 5 letters A-Z).
    void build(const SafeArray<string>& words);
    void build(const SinglyLinkedList<string>& words);

    // Adds one word to the counts. Words that are not 5 letters A-Z are skipped.
    void add(const string& word);
    void clear();

    // Throw an error for a position outside 0-4 or a non-letter.
    int countAt(int position, char letter) const;
    int wordsContaining(char letter) const;
    int wordCount() const { return wordCount_; }

    // Score of a word: for each position, how many words share that letter
    // there, plus, for each distinct letter, how many words contain it.
    // Repeated letters only score once, so words that test five different
    // letters rank higher.
    long long score(const string& word) const;

private:
    int** counts_;
    int totals_[kLetters];
    int wordCount_;

    void allocate();
    void release();
    void copyFrom(const FrequencyTable& other);
    static int letterIndex(char letter); // -1 for a non-letter
};

#endif // WORDLE_DSA_FREQUENCY_TABLE_H
