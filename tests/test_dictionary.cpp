#include "TestFramework.h"
#include "Dictionary.h"
#include "WordPicker.h"
#include "ds/HashTable.h"

#include <sstream>
#include <stdexcept>
#include <string>

#ifndef WORDLE_DATA_DIR
#define WORDLE_DATA_DIR "data"
#endif

static const std::string kWordsFile = std::string(WORDLE_DATA_DIR) + "/words.txt";

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static Dictionary smallDictionary()
{
    std::istringstream input("slate\nCRANE\napple\nbrave\nplumb\nzebra\nabout\n");
    Dictionary dictionary;
    dictionary.loadFromStream(input);
    return dictionary;
}

static Dictionary realDictionary()
{
    Dictionary dictionary;
    dictionary.loadFromFile(kWordsFile);
    return dictionary;
}

static SafeArray<std::string> wordsOf(std::initializer_list<const char*> list)
{
    SafeArray<std::string> words;
    for (const char* w : list)
        words.pushBack(w);
    return words;
}

// ===========================================================================
// Loading
// ===========================================================================

TEST_CASE(dictionaryLoadsTheRealWordList)
{
    Dictionary dictionary = realDictionary();
    CHECK(dictionary.size() > 500u);
    CHECK(dictionary.isInDictionary("CRANE"));
    CHECK(dictionary.isInDictionary("apple"));
}

TEST_CASE(dictionaryMissingFileThrows)
{
    Dictionary dictionary;
    CHECK_THROWS(dictionary.loadFromFile("does/not/exist/words.txt"), std::runtime_error);
    CHECK(dictionary.empty());
}

TEST_CASE(dictionaryEmptyInputThrows)
{
    std::istringstream input("");
    Dictionary dictionary;
    CHECK_THROWS(dictionary.loadFromStream(input), std::runtime_error);
    CHECK(dictionary.empty());
}

TEST_CASE(dictionaryInputWithNoValidWordThrows)
{
    std::istringstream input("cat\nelephant\n12345\nab-cd\n\n");
    Dictionary dictionary;
    CHECK_THROWS(dictionary.loadFromStream(input), std::runtime_error);
}

TEST_CASE(dictionarySkipsBadLinesAndDuplicates)
{
    std::istringstream input("crane\n  CRANE \ncrane\r\ncat\nslate\nsl@te\n\nSlAtE\n");
    Dictionary dictionary;
    CHECK_EQ(dictionary.loadFromStream(input), 2u);
    CHECK(dictionary.isInDictionary("CRANE"));
    CHECK(dictionary.isInDictionary("SLATE"));
    CHECK(!dictionary.isInDictionary("CAT"));
}

TEST_CASE(dictionaryReloadReplacesOldWords)
{
    Dictionary dictionary = smallDictionary();
    std::istringstream input("ghost\n");
    dictionary.loadFromStream(input);
    CHECK_EQ(dictionary.size(), 1u);
    CHECK(dictionary.isInDictionary("GHOST"));
    CHECK(!dictionary.isInDictionary("CRANE"));
}

TEST_CASE(dictionaryWordsAreRadixSorted)
{
    Dictionary dictionary = realDictionary();
    const SafeArray<std::string>& words = dictionary.sortedWords();
    for (std::size_t i = 1; i < words.size(); ++i)
        CHECK(words[i - 1] < words[i]);
}

TEST_CASE(dictionaryAvlInOrderMatchesSortedArray)
{
    CHECK(smallDictionary().treeMatchesSortedWords());
    CHECK(realDictionary().treeMatchesSortedWords());
}

// ===========================================================================
// Word checks
// ===========================================================================

TEST_CASE(dictionaryNormalizeTrimsAndUpperCases)
{
    CHECK_EQ(Dictionary::normalize("  crane\r\n"), std::string("CRANE"));
    CHECK_EQ(Dictionary::normalize("SlAtE"), std::string("SLATE"));
    CHECK_EQ(Dictionary::normalize(""), std::string(""));
}

TEST_CASE(dictionaryIsWellFormed)
{
    CHECK(Dictionary::isWellFormed("crane"));
    CHECK(Dictionary::isWellFormed(" CRANE "));
    CHECK(!Dictionary::isWellFormed("cran"));
    CHECK(!Dictionary::isWellFormed("cranes"));
    CHECK(!Dictionary::isWellFormed("cr4ne"));
    CHECK(!Dictionary::isWellFormed(""));
}

TEST_CASE(dictionaryLookupIsCaseInsensitive)
{
    Dictionary dictionary = smallDictionary();
    CHECK(dictionary.isInDictionary("crane"));
    CHECK(dictionary.isInDictionary("Crane"));
    CHECK(dictionary.isInDictionary("CRANE"));
    CHECK(!dictionary.isInDictionary("CRANK"));
    CHECK(!dictionary.isInDictionary("CRAN"));
}

// ===========================================================================
// Five lookup methods
// ===========================================================================

TEST_CASE(dictionaryAllSixMethodsAgreeOnEveryWord)
{
    Dictionary dictionary = realDictionary();
    for (const std::string& word : dictionary.sortedWords())
    {
        LookupStats stats = dictionary.lookupAll(word);
        CHECK(stats.allAgree());
        CHECK(stats.hash.found);
        CHECK(stats.btree.found);
    }
}

TEST_CASE(dictionaryAllSixMethodsAgreeOnMissingWords)
{
    Dictionary dictionary = realDictionary();
    const char* missing[] = {"AAAAA", "ZZZZZ", "CRANK", "QWERT", "MMMMM", "BBBBB"};
    for (const char* word : missing)
    {
        LookupStats stats = dictionary.lookupAll(word);
        CHECK(stats.allAgree());
        CHECK(!stats.hash.found);
    }
}

TEST_CASE(dictionaryLookupCountsComparisons)
{
    Dictionary dictionary = realDictionary();
    LookupStats stats = dictionary.lookupAll("zebra"); // near the end of the list
    CHECK_EQ(stats.word, std::string("ZEBRA"));
    CHECK(stats.linear.found);
    CHECK(stats.linear.comparisons > 100);              // O(n): walks almost every word
    CHECK(stats.binary.comparisons <= 11);              // ceil(log2(701)) = 10, +1 slack
    CHECK(stats.binary.comparisons < stats.linear.comparisons);
    CHECK(stats.avl.comparisons >= 1);
    CHECK(stats.hash.comparisons >= 1);
    CHECK(stats.hash.comparisons <= static_cast<int>(dictionary.stats().hashLongestChain));
    CHECK(stats.btree.found);
    CHECK(stats.btreeNodesVisited <= dictionary.stats().btreeHeight);
}

TEST_CASE(dictionaryBTreeMatchesSortedArray)
{
    CHECK(smallDictionary().btreeMatchesSortedWords());
    CHECK(realDictionary().btreeMatchesSortedWords());
}

TEST_CASE(dictionaryBTreeIsShorterThanAvl)
{
    DictionaryStats s = realDictionary().stats();
    CHECK(s.btreeHeight >= 1);
    CHECK(s.btreeHeight < s.avlHeight);
    CHECK(s.btreeNodes < s.wordCount); // 1-2 words per node
    CHECK(s.avlBalanced);
    CHECK(s.avlLeaves > 0u);
}

TEST_CASE(dictionaryLookupOfMalformedWordFindsNothing)
{
    Dictionary dictionary = smallDictionary();
    LookupStats stats = dictionary.lookupAll("toolong");
    CHECK(!stats.linear.found);
    CHECK(!stats.hash.found);
    CHECK_EQ(stats.linear.comparisons, 0);
    CHECK(stats.allAgree());
}

TEST_CASE(dictionaryStatsDescribeTheStructures)
{
    Dictionary dictionary = realDictionary();
    DictionaryStats s = dictionary.stats();
    CHECK_EQ(s.wordCount, dictionary.size());
    CHECK_EQ(s.hashSize, dictionary.size());
    CHECK(s.hashLoadFactor <= 0.75);
    CHECK(s.hashRehashCount > 0u);
    // AVL height is at most about 1.44 * log2(n): 701 words -> well under 14.
    CHECK(s.avlHeight >= 10);
    CHECK(s.avlHeight <= 14);
}

// ===========================================================================
// WordPicker
// ===========================================================================

TEST_CASE(wordPickerNoRepeatsWithinOneCycle)
{
    Dictionary dictionary = realDictionary();
    WordPicker picker(dictionary.sortedWords(), 42u);

    HashTable<std::string, bool> seen;
    for (std::size_t i = 0; i < picker.size(); ++i)
    {
        std::string word = picker.nextWord();
        CHECK(dictionary.isInDictionary(word));
        CHECK(seen.insert(word, true)); // false would mean a repeat
    }
    CHECK_EQ(seen.size(), dictionary.size());
    CHECK_EQ(picker.completedCycles(), 1u);
}

TEST_CASE(wordPickerReshufflesAfterAFullCycle)
{
    SafeArray<std::string> words = wordsOf({"BRAVE", "CRANE", "SLATE", "PLUMB", "GHOST"});
    WordPicker picker(words, 7u);

    for (int cycle = 0; cycle < 4; ++cycle)
    {
        HashTable<std::string, bool> seen;
        for (int i = 0; i < 5; ++i)
            CHECK(seen.insert(picker.nextWord(), true));
        CHECK_EQ(picker.completedCycles(), static_cast<std::size_t>(cycle + 1));
        CHECK_EQ(picker.servedThisCycle(), 0u);
    }
}

TEST_CASE(wordPickerNeverRepeatsAcrossTheCycleBoundary)
{
    SafeArray<std::string> words = wordsOf({"BRAVE", "CRANE", "SLATE"});
    WordPicker picker(words, 3u);
    std::string previous = picker.nextWord();
    for (int i = 0; i < 300; ++i)
    {
        std::string current = picker.nextWord();
        CHECK(current != previous);
        previous = current;
    }
}

TEST_CASE(wordPickerSameSeedGivesSameOrder)
{
    SafeArray<std::string> words = wordsOf({"BRAVE", "CRANE", "SLATE", "PLUMB", "GHOST"});
    WordPicker a(words, 123u);
    WordPicker b(words, 123u);
    for (int i = 0; i < 12; ++i)
        CHECK_EQ(a.nextWord(), b.nextWord());
}

TEST_CASE(wordPickerSingleWordRepeatsIt)
{
    WordPicker picker(wordsOf({"CRANE"}), 1u);
    CHECK_EQ(picker.nextWord(), std::string("CRANE"));
    CHECK_EQ(picker.nextWord(), std::string("CRANE"));
}

TEST_CASE(wordPickerEmptyListThrows)
{
    SafeArray<std::string> none;
    WordPicker picker(none, 1u);
    CHECK_THROWS(picker.nextWord(), std::logic_error);
}

int main()
{
    return tf::runAll("Dictionary and WordPicker");
}
