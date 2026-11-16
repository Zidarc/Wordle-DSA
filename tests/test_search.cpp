#include "TestFramework.h"
#include "algorithms/Search.h"

#include <string>

TEST_CASE(wordToKeyOrdersWordsLikeTheAlphabet)
{
    CHECK_EQ(wordToKey("AAAAA"), 0L);
    CHECK_EQ(wordToKey("AAAAB"), 1L);
    CHECK_EQ(wordToKey("AAABA"), 26L);
    CHECK(wordToKey("APPLE") < wordToKey("CRANE"));
    CHECK_EQ(wordToKey("crane"), wordToKey("CRANE"));
}

TEST_CASE(linearSearchFindsFirstMiddleLast)
{
    std::string words[] = {"APPLE", "BRAVE", "CRANE", "PLUMB", "SLATE"};
    CHECK_EQ(linearSearch(words, 5, std::string("APPLE")), 0);
    CHECK_EQ(linearSearch(words, 5, std::string("CRANE")), 2);
    CHECK_EQ(linearSearch(words, 5, std::string("SLATE")), 4);
}

TEST_CASE(linearSearchMissingAndEmpty)
{
    std::string words[] = {"APPLE", "BRAVE", "CRANE"};
    CHECK_EQ(linearSearch(words, 3, std::string("ZZZZZ")), -1);
    CHECK_EQ(linearSearch(words, 0, std::string("APPLE")), -1);
}

TEST_CASE(linearSearchCountsComparisons)
{
    std::string words[] = {"APPLE", "BRAVE", "CRANE", "PLUMB", "SLATE"};
    int count = 0;
    linearSearch(words, 5, std::string("CRANE"), &count);
    CHECK_EQ(count, 3);

    count = 0;
    linearSearch(words, 5, std::string("ZZZZZ"), &count);
    CHECK_EQ(count, 5);
}

TEST_CASE(binarySearchFindsFirstMiddleLast)
{
    std::string words[] = {"APPLE", "BRAVE", "CRANE", "PLUMB", "SLATE"};
    CHECK_EQ(binarySearch(words, 0, 4, std::string("APPLE")), 0);
    CHECK_EQ(binarySearch(words, 0, 4, std::string("CRANE")), 2);
    CHECK_EQ(binarySearch(words, 0, 4, std::string("SLATE")), 4);
}

TEST_CASE(binarySearchMissingAndEmpty)
{
    std::string words[] = {"APPLE", "BRAVE", "CRANE", "PLUMB", "SLATE"};
    CHECK_EQ(binarySearch(words, 0, 4, std::string("AAAAA")), -1);
    CHECK_EQ(binarySearch(words, 0, 4, std::string("DRIVE")), -1);
    CHECK_EQ(binarySearch(words, 0, 4, std::string("ZZZZZ")), -1);
    CHECK_EQ(binarySearch(words, 0, -1, std::string("APPLE")), -1);
}

TEST_CASE(binarySearchCountsFewComparisons)
{
    std::string words[] = {"APPLE", "BRAVE", "CRANE", "PLUMB", "SLATE", "TRAIN", "WORLD"};
    int count = 0;
    CHECK_EQ(binarySearch(words, 0, 6, std::string("SLATE"), &count), 4);
    CHECK(count <= 3);
}

TEST_CASE(interpolationSearchFindsFirstMiddleLast)
{
    std::string words[] = {"APPLE", "BRAVE", "CRANE", "PLUMB", "SLATE"};
    CHECK_EQ(interpolationSearch(words, 5, std::string("APPLE")), 0);
    CHECK_EQ(interpolationSearch(words, 5, std::string("CRANE")), 2);
    CHECK_EQ(interpolationSearch(words, 5, std::string("SLATE")), 4);
}

TEST_CASE(interpolationSearchMissingAndEmpty)
{
    std::string words[] = {"APPLE", "BRAVE", "CRANE", "PLUMB", "SLATE"};
    CHECK_EQ(interpolationSearch(words, 5, std::string("AAAAA")), -1);
    CHECK_EQ(interpolationSearch(words, 5, std::string("DRIVE")), -1);
    CHECK_EQ(interpolationSearch(words, 5, std::string("ZZZZZ")), -1);
    CHECK_EQ(interpolationSearch(words, 0, std::string("APPLE")), -1);
}

TEST_CASE(interpolationSearchSingleWord)
{
    std::string words[] = {"CRANE"};
    CHECK_EQ(interpolationSearch(words, 1, std::string("CRANE")), 0);
    CHECK_EQ(interpolationSearch(words, 1, std::string("BRAVE")), -1);
}

TEST_CASE(allThreeSearchesAgree)
{
    std::string words[] = {"APPLE", "BRAVE", "CRANE", "GRAPE", "PLUMB", "SLATE", "TRAIN", "WORLD"};
    std::string probes[] = {"APPLE", "GRAPE", "WORLD", "SLATE", "ABOUT", "ZEBRA", "DRIVE"};

    for (int i = 0; i < 7; i++)
    {
        int a = linearSearch(words, 8, probes[i]);
        int b = binarySearch(words, 0, 7, probes[i]);
        int c = interpolationSearch(words, 8, probes[i]);
        CHECK_EQ(a, b);
        CHECK_EQ(b, c);
    }
}

int main()
{
    return tf::runAll("Search");
}