#ifndef WORDLE_DSA_WORD_PICKER_H
#define WORDLE_DSA_WORD_PICKER_H

// Chooses the secret word for each new game.
//
// The words are shuffled (Fisher-Yates) into a circular linked list. Each new
// game takes the word under the cursor and moves the cursor one step, so no
// word repeats until every word has been used. After a full lap, the words
// are reshuffled for the next cycle.
//
//    ┌──► BRAVE ──► CRANE ──► SLATE ──► PLUMB ──┐
//    └──────────────────────────────────────────┘
//                ↑
//             cursor

#include "ds/CircularLinkedList.h"
#include "ds/SafeArray.h"

#include <cstddef>
#include <random>
#include <string>

using namespace std;

class WordPicker
{
public:
    // Uses a random seed taken from the clock.
    explicit WordPicker(const SafeArray<string>& words);

    // Fixed seed, so tests get the same order every run.
    WordPicker(const SafeArray<string>& words, unsigned int seed);

    // Returns the next secret word and moves the cursor forward.
    // Throws an error if there are no words.
    string nextWord();

    size_t size() const { return words_.size(); }

    // How many words have been handed out in the current cycle (0 .. size()-1).
    size_t servedThisCycle() const { return servedThisCycle_; }

    // How many full cycles have been completed (each one ends with a reshuffle).
    size_t completedCycles() const { return completedCycles_; }

private:
    SafeArray<string> words_;
    CircularLinkedList<string> rotation_;
    mt19937 generator_;
    size_t servedThisCycle_ = 0;
    size_t completedCycles_ = 0;
    string lastServed_;

    void startNewCycle();
};

#endif // WORDLE_DSA_WORD_PICKER_H
