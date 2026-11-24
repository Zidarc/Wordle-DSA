// Tests for the recursion examples in Recursion.h.

#include "TestFramework.h"

#include "algorithms/Recursion.h"

#include <string>

TEST_CASE(tailRecursionCountsLetters)
{
    CHECK_EQ(countLetterTail("APPLE", 'P'), 2);
    CHECK_EQ(countLetterTail("APPLE", 'Z'), 0);
    CHECK_EQ(countLetterTail("", 'A'), 0);
    CHECK_EQ(countLetterTail("PUPPY", 'P'), 3);
}

TEST_CASE(headRecursionReverses)
{
    CHECK_EQ(reverseHead("CRANE"), std::string("ENARC"));
    CHECK_EQ(reverseHead("A"), std::string("A"));
    CHECK_EQ(reverseHead(""), std::string(""));
}

TEST_CASE(treeRecursionCountsOrderings)
{
    CHECK_EQ(countOrderings(0, 26), 1LL);
    CHECK_EQ(countOrderings(2, 26), 676LL);
    CHECK_EQ(countOrderings(3, 4), 64LL);
}

TEST_CASE(naiveFibonacciRepeatsWork)
{
    long long calls = 0;
    CHECK_EQ(fibonacciTree(10, calls), 55LL);
    CHECK_EQ(calls, 177LL); // 2 * fib(11) - 1 calls for only 11 distinct values
}

TEST_CASE(indirectRecursionAlternatingWords)
{
    CHECK(isAlternatingWord("BANAL"));  // B A N A L
    CHECK(isAlternatingWord("AROSE"));  // A R O S E
    CHECK(!isAlternatingWord("CRANE")); // C R: two consonants
    CHECK(!isAlternatingWord("AUDIO"));
    CHECK(isAlternatingWord(""));
}

TEST_CASE(nestedRecursionMcCarthy91)
{
    CHECK_EQ(mcCarthy91(1), 91);
    CHECK_EQ(mcCarthy91(100), 91);
    CHECK_EQ(mcCarthy91(101), 91);
    CHECK_EQ(mcCarthy91(150), 140);
}

int main()
{
    return tf::runAll("Recursion types");
}
