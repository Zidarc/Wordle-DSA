#ifndef WORDLE_DSA_DICTIONARY_H
#define WORDLE_DSA_DICTIONARY_H

// The word list, and every way the game can look a word up.
//
// On load, the words are cleaned (trimmed, upper-cased, only 5-letter A-Z
// words kept, duplicates dropped), radix-sorted, and then stored three ways:
//
//   sortedWords_  SafeArray, sorted   -> linear, binary, interpolation search
//   tree_         AVL tree            -> O(log n) lookup
//   table_        hash table          -> O(1) average lookup (the one the game uses)
//
// lookupAll() runs the same guess through all five methods and reports how
// many comparisons each one needed, for the DSA Insights panel.

#include "ds/AVLTree.h"
#include "ds/HashTable.h"
#include "ds/SafeArray.h"

#include <cstddef>
#include <istream>
#include <string>

// Result of one lookup method: did it find the word, and how many comparisons it took.
struct LookupResult
{
    bool found = false;
    int comparisons = 0;
};

// The same word checked by all five methods.
struct LookupStats
{
    std::string word; // normalised (upper-case) form of the input
    LookupResult linear;
    LookupResult binary;
    LookupResult interpolation;
    LookupResult avl;
    LookupResult hash;

    // True when all five methods agree (they always should).
    bool allAgree() const;
};

// Sizes and shape of the structures, for GET /api/dsa/stats.
struct DictionaryStats
{
    std::size_t wordCount = 0;
    int avlHeight = 0;
    std::size_t hashSize = 0;
    std::size_t hashBuckets = 0;
    std::size_t hashUsedBuckets = 0;
    std::size_t hashLongestChain = 0;
    std::size_t hashRehashCount = 0;
    double hashLoadFactor = 0.0;
};

class Dictionary
{
public:
    static constexpr int kWordLength = 5;

    Dictionary() = default;

    // Loads the words from a file, one word per line. Replaces any words already loaded.
    // Throws std::runtime_error if the file cannot be opened or holds no valid word.
    // Returns the number of words loaded.
    std::size_t loadFromFile(const std::string& path);

    // Same as loadFromFile, but reads from any stream (used by the tests).
    std::size_t loadFromStream(std::istream& input);

    // Upper-cases the word and trims surrounding whitespace. Does not validate it.
    static std::string normalize(const std::string& word);

    // True if the word has exactly 5 letters A-Z (after normalising).
    static bool isWellFormed(const std::string& word);

    // The check the game relies on: a hash table lookup. Case-insensitive.
    bool isInDictionary(const std::string& word) const;

    // Runs all five lookup methods on the same word and counts their comparisons.
    LookupStats lookupAll(const std::string& word) const;

    DictionaryStats stats() const;

    std::size_t size() const { return sortedWords_.size(); }
    bool empty() const { return sortedWords_.empty(); }

    // All words in sorted order (used to build the WordPicker).
    const SafeArray<std::string>& sortedWords() const { return sortedWords_; }

    // Checks the AVL tree with an iterative in-order traversal (our own Stack):
    // true if it returns exactly the sorted array.
    bool treeMatchesSortedWords() const;

private:
    SafeArray<std::string> sortedWords_;
    AVLTree<std::string> tree_;
    HashTable<std::string, bool> table_;

    void clear();
};

#endif // WORDLE_DSA_DICTIONARY_H
