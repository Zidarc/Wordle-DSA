#include "TestFramework.h"
#include "algorithms/StringSearch.h"

#include <string>

TEST_CASE(buildLpsMatchesKnownTables)
{
    int a[5];
    buildLps("ABABC", a);
    CHECK_EQ(a[0], 0);
    CHECK_EQ(a[1], 0);
    CHECK_EQ(a[2], 1);
    CHECK_EQ(a[3], 2);
    CHECK_EQ(a[4], 0);

    int b[4];
    buildLps("AAAA", b);
    CHECK_EQ(b[0], 0);
    CHECK_EQ(b[1], 1);
    CHECK_EQ(b[2], 2);
    CHECK_EQ(b[3], 3);
}

TEST_CASE(kmpFindsMatchAtStartMiddleEnd)
{
    CHECK_EQ(kmpSearch("CRANEBRAVE", "CRANE"), 0);
    CHECK_EQ(kmpSearch("XXCRANEXX", "CRANE"), 2);
    CHECK_EQ(kmpSearch("BRAVECRANE", "CRANE"), 5);
}

TEST_CASE(kmpNoMatch)
{
    CHECK_EQ(kmpSearch("BRAVE", "CRANE"), -1);
    CHECK_EQ(kmpSearch("CRAN", "CRANE"), -1);
    CHECK_EQ(kmpSearch("", "A"), -1);
    CHECK(!kmpContains("Ali Hussain", "xyz"));
}

TEST_CASE(kmpHandlesEmptyPattern)
{
    CHECK_EQ(kmpSearch("CRANE", ""), 0);
    CHECK(kmpContains("CRANE", ""));
    CHECK_EQ(kmpCountMatches("CRANE", ""), 0);
}

TEST_CASE(kmpHandlesRepeatedPrefixes)
{
    CHECK_EQ(kmpSearch("ABABABC", "ABABC"), 2);
    CHECK_EQ(kmpSearch("AAAAAB", "AAB"), 3);
}

TEST_CASE(kmpCountsOverlappingMatches)
{
    CHECK_EQ(kmpCountMatches("AAAAA", "AA"), 4);
    CHECK_EQ(kmpCountMatches("ABABABA", "ABA"), 3);
    CHECK_EQ(kmpCountMatches("CRANE", "X"), 0);
}

TEST_CASE(kmpSearchesPlayerNamesIgnoringCase)
{
    CHECK(kmpContains("Ali Hussain", "hus", true));
    CHECK(!kmpContains("Ali Hussain", "hus"));
    CHECK(kmpContains("Ali Hussain", "Hus"));
    CHECK_EQ(kmpSearch("M. Abbas", "ABB", nullptr, true), 3);
}

TEST_CASE(kmpComparisonCountStaysLinear)
{
    std::string text = "AAAAAAAAAAAAAAAAAAAB";
    int count = 0;
    CHECK_EQ(kmpSearch(text, "AAAB", &count), 16);
    CHECK(count > 0);
    CHECK(count <= 2 * (int)text.length());
}

int main()
{
    return tf::runAll("KMP");
}