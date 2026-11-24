#include "Dictionary.h"

#include "algorithms/Search.h"
#include "algorithms/Sort.h"

#include <fstream>
#include <stdexcept>

bool LookupStats::allAgree() const
{
    return linear.found == binary.found && binary.found == interpolation.found &&
           interpolation.found == avl.found && avl.found == btree.found &&
           btree.found == hash.found;
}

// ---------------------------------------------------------------------------
// Loading
// ---------------------------------------------------------------------------

std::size_t Dictionary::loadFromFile(const std::string& path)
{
    std::ifstream file(path);
    if (!file)
        throw std::runtime_error("Dictionary: cannot open word list '" + path + "'");
    return loadFromStream(file);
}

std::size_t Dictionary::loadFromStream(std::istream& input)
{
    clear();

    // Read and clean every line. The hash table doubles as the duplicate check.
    std::string line;
    while (std::getline(input, line))
    {
        std::string word = normalize(line);
        if (!isWellFormed(word))
            continue;
        if (table_.insert(word, true))
            sortedWords_.pushBack(word);
    }

    if (sortedWords_.empty())
        throw std::runtime_error("Dictionary: the word list holds no valid 5-letter word");

    // Every word is 5 letters from a 26-letter alphabet, so LSD radix sort fits:
    // O(5 * (n + 26)).
    radixSortWords(sortedWords_, static_cast<int>(sortedWords_.size()), kWordLength);

    for (const std::string& word : sortedWords_)
    {
        tree_.insert(word);
        btree_.insert(word);
    }

    return sortedWords_.size();
}

void Dictionary::clear()
{
    sortedWords_.clear();
    tree_.clear();
    btree_.clear();
    table_.clear();
}

// ---------------------------------------------------------------------------
// Word checks
// ---------------------------------------------------------------------------

std::string Dictionary::normalize(const std::string& word)
{
    std::size_t first = 0;
    std::size_t last = word.size();
    while (first < last && (word[first] == ' ' || word[first] == '\t' || word[first] == '\r' ||
                            word[first] == '\n'))
        ++first;
    while (last > first && (word[last - 1] == ' ' || word[last - 1] == '\t' ||
                            word[last - 1] == '\r' || word[last - 1] == '\n'))
        --last;

    std::string result = word.substr(first, last - first);
    for (char& c : result)
    {
        if (c >= 'a' && c <= 'z')
            c = static_cast<char>(c - 'a' + 'A');
    }
    return result;
}

bool Dictionary::isWellFormed(const std::string& word)
{
    std::string w = normalize(word);
    if (static_cast<int>(w.size()) != kWordLength)
        return false;
    for (char c : w)
    {
        if (c < 'A' || c > 'Z')
            return false;
    }
    return true;
}

bool Dictionary::isInDictionary(const std::string& word) const
{
    std::string w = normalize(word);
    if (!isWellFormed(w))
        return false;
    return table_.contains(w);
}

LookupStats Dictionary::lookupAll(const std::string& word) const
{
    LookupStats stats;
    stats.word = normalize(word);
    if (!isWellFormed(stats.word) || sortedWords_.empty())
        return stats; // not a 5-letter word: every method reports "not found", 0 comparisons

    const int n = static_cast<int>(sortedWords_.size());
    const std::string& w = stats.word;

    stats.linear.found =
        linearSearch(sortedWords_, n, w, &stats.linear.comparisons) != -1;
    stats.binary.found =
        binarySearch(sortedWords_, 0, n - 1, w, &stats.binary.comparisons) != -1;
    stats.interpolation.found =
        interpolationSearch(sortedWords_, n, w, &stats.interpolation.comparisons) != -1;
    stats.avl.found = tree_.contains(w, stats.avl.comparisons);
    stats.btree.found = btree_.search(w, stats.btree.comparisons, stats.btreeNodesVisited);
    stats.hash.found = table_.contains(w, stats.hash.comparisons);

    return stats;
}

// ---------------------------------------------------------------------------
// Statistics
// ---------------------------------------------------------------------------

DictionaryStats Dictionary::stats() const
{
    DictionaryStats s;
    s.wordCount = sortedWords_.size();
    s.avlHeight = tree_.height();
    s.avlLeaves = tree_.leafCount();
    s.avlBalanced = tree_.isBalanced();
    s.btreeHeight = btree_.height();
    s.btreeNodes = btree_.nodeCount();
    s.hashSize = table_.size();
    s.hashBuckets = table_.bucketCount();
    s.hashUsedBuckets = table_.usedBuckets();
    s.hashLongestChain = table_.longestChain();
    s.hashRehashCount = table_.rehashCount();
    s.hashLoadFactor = table_.loadFactor();
    return s;
}

bool Dictionary::treeMatchesSortedWords() const
{
    SafeArray<std::string> inOrder;
    tree_.toSortedArray(inOrder);
    if (inOrder.size() != sortedWords_.size())
        return false;
    for (std::size_t i = 0; i < inOrder.size(); ++i)
    {
        if (inOrder[i] != sortedWords_[i])
            return false;
    }
    return true;
}

bool Dictionary::btreeMatchesSortedWords() const
{
    if (!btree_.isValid())
        return false;
    SafeArray<std::string> inOrder;
    btree_.toSortedArray(inOrder);
    if (inOrder.size() != sortedWords_.size())
        return false;
    for (std::size_t i = 0; i < inOrder.size(); ++i)
    {
        if (inOrder[i] != sortedWords_[i])
            return false;
    }
    return true;
}
