#include "Dictionary.h"

#include "algorithms/Search.h"
#include "algorithms/Sort.h"

#include <chrono>
#include <fstream>
#include <stdexcept>

bool LookupStats::allAgree() const
{
    return linear.found == binary.found && binary.found == interpolation.found &&
           interpolation.found == avl.found && avl.found == btree.found &&
           btree.found == hash.found;
}

const LookupResult& LookupStats::get(LookupMethod method) const
{
    switch (method)
    {
    case LookupMethod::Linear:
        return linear;
    case LookupMethod::Binary:
        return binary;
    case LookupMethod::Interpolation:
        return interpolation;
    case LookupMethod::Avl:
        return avl;
    case LookupMethod::BTree:
        return btree;
    case LookupMethod::Hash:
    default:
        return hash;
    }
}

// ---------------------------------------------------------------------------
// Lookup methods: names and complexities (n = number of words, m = buckets)
// ---------------------------------------------------------------------------

namespace
{
// GUIDE 2.1: the time / space table shown in DSA Insights
const LookupMethodInfo kMethods[Dictionary::kMethodCount] = {
    {LookupMethod::Linear, "linear", "Linear search", "O(1)", "O(n)", "O(n)",
     "O(n) sorted array", "O(1)", "SafeArray (sorted)"},
    {LookupMethod::Binary, "binary", "Binary search", "O(1)", "O(log n)", "O(log n)",
     "O(n) sorted array", "O(log n) recursion stack", "SafeArray (sorted)"},
    {LookupMethod::Interpolation, "interpolation", "Interpolation search", "O(1)",
     "O(log log n)", "O(n)", "O(n) sorted array", "O(1)", "SafeArray (sorted, base-26 keys)"},
    {LookupMethod::Avl, "avl", "AVL tree", "O(1)", "O(log n)", "O(log n)",
     "O(n) nodes", "O(log n) recursion stack", "AVLTree<string>"},
    {LookupMethod::BTree, "btree", "2-3 tree (B-tree)", "O(1)", "O(log n)", "O(log n)",
     "O(n) nodes", "O(1) (iterative)", "BTree<string, 3>"},
    {LookupMethod::Hash, "hash", "Hash table", "O(1)", "O(1)", "O(n)",
     "O(n + m) chains + buckets", "O(1)", "HashTable<string, bool> (chaining)"},
};
} // namespace

const LookupMethodInfo& Dictionary::methodInfo(int index)
{
    if (index < 0 || index >= kMethodCount)
        throw std::out_of_range("Dictionary::methodInfo: index out of range");
    return kMethods[index];
}

const LookupMethodInfo& Dictionary::methodInfo(LookupMethod method)
{
    return methodInfo(static_cast<int>(method));
}

bool Dictionary::methodFromString(const std::string& name, LookupMethod& method)
{
    std::string lower = name;
    for (char& c : lower)
    {
        if (c >= 'A' && c <= 'Z')
            c = static_cast<char>(c - 'A' + 'a');
    }
    for (const LookupMethodInfo& info : kMethods)
    {
        if (lower == info.key)
        {
            method = info.method;
            return true;
        }
    }
    return false;
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

// GUIDE 3.1: hash table (dedupe) -> radix sort -> AVL + 2-3 tree
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

// Runs `lookup` once to record its answer and comparison count, then
// kTimingRepeats more times to measure the average time of one call.
// TIP: this is where the "ns" numbers in the DSA Insights panel come from.
template <typename Lookup>
static LookupResult timedLookup(Lookup lookup, int repeats)
{
    LookupResult result;
    result.found = lookup(result.comparisons);

    int ignored = 0;
    volatile bool sink = false; // volatile: the loop cannot be optimised away
    auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < repeats; ++i)
        sink = lookup(ignored);
    auto end = std::chrono::steady_clock::now();
    (void)sink;

    long long total = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    result.nanoseconds = total / repeats;
    return result;
}

LookupResult Dictionary::lookupWith(const std::string& word, LookupMethod method) const
{
    // All six run anyway for the comparison table; pick the chosen one.
    return lookupAll(word).get(method);
}

// GUIDE 2.1 / 3.3: all six lookups on one word, each timed
LookupStats Dictionary::lookupAll(const std::string& word) const
{
    LookupStats stats;
    stats.word = normalize(word);
    if (!isWellFormed(stats.word) || sortedWords_.empty())
        return stats; // not a 5-letter word: every method reports "not found", 0 comparisons

    const int n = static_cast<int>(sortedWords_.size());
    const std::string& w = stats.word;
    const int r = kTimingRepeats;

    stats.linear = timedLookup([&](int& c) { return linearSearch(sortedWords_, n, w, &c) != -1; }, r);
    stats.binary = timedLookup([&](int& c) { return binarySearch(sortedWords_, 0, n - 1, w, &c) != -1; }, r);
    stats.interpolation =
        timedLookup([&](int& c) { return interpolationSearch(sortedWords_, n, w, &c) != -1; }, r);
    stats.avl = timedLookup([&](int& c) { return tree_.contains(w, c); }, r);
    stats.btree = timedLookup(
        [&](int& c) {
            int nodes = 0;
            bool found = btree_.search(w, c, nodes);
            stats.btreeNodesVisited = nodes;
            return found;
        },
        r);
    stats.hash = timedLookup([&](int& c) { return table_.contains(w, c); }, r);

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
