#include "TestFramework.h"
#include "algorithms/Shuffle.h"

#include <string>

TEST_CASE(shuffleKeepsEveryElementExactlyOnce)
{
    int a[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    shuffleArray(a, 10, 42u);

    int seen[11] = {0};
    int sum = 0;
    for (int i = 0; i < 10; i++)
    {
        seen[a[i]]++;
        sum += a[i];
    }
    CHECK_EQ(sum, 55);
    for (int v = 1; v <= 10; v++)
        CHECK_EQ(seen[v], 1);
}

TEST_CASE(shuffleEmptyAndOneElement)
{
    int empty[1] = {0};
    shuffleArray(empty, 0, 1u);

    int one[] = {7};
    shuffleArray(one, 1, 1u);
    CHECK_EQ(one[0], 7);
}

TEST_CASE(sameSeedGivesSameOrder)
{
    int a[] = {1, 2, 3, 4, 5, 6, 7, 8};
    int b[] = {1, 2, 3, 4, 5, 6, 7, 8};
    shuffleArray(a, 8, 123u);
    shuffleArray(b, 8, 123u);
    for (int i = 0; i < 8; i++)
        CHECK_EQ(a[i], b[i]);
}

TEST_CASE(differentSeedsCanChangeTheOrder)
{
    bool changed = false;
    for (unsigned int seed = 1; seed <= 20; seed++)
    {
        int a[] = {1, 2, 3, 4, 5, 6, 7, 8};
        shuffleArray(a, 8, seed);
        for (int i = 0; i < 8; i++)
        {
            if (a[i] != i + 1)
                changed = true;
        }
    }
    CHECK(changed);
}

TEST_CASE(everyElementCanLandInFirstPlace){
    bool landedFirst[5] = {false, false, false, false, false};
    for (unsigned int seed = 1; seed <= 200; seed++)
    {
        int positions[] = {0, 1, 2, 3, 4};
        shuffleArray(positions, 5, seed);
        landedFirst[positions[0]] = true;
    }
    for (int p = 0; p < 5; p++)
        CHECK(landedFirst[p]);
}

TEST_CASE(shuffleKeepsWordsIntact)
{
    std::string words[] = {"CRANE", "BRAVE", "SLATE", "PLUMB", "APPLE"};
    shuffleArray(words, 5, 7u);

    const char* original[] = {"CRANE", "BRAVE", "SLATE", "PLUMB", "APPLE"};
    for (int k = 0; k < 5; k++)
    {
        int found = 0;
        for (int i = 0; i < 5; i++)
        {
            if (words[i] == original[k])
                found++;
        }
        CHECK_EQ(found, 1);
    }
}

TEST_CASE(unseededShuffleKeepsEveryElement)
{
    int a[] = {1, 2, 3, 4, 5, 6};
    shuffleArray(a, 6);

    int sum = 0;
    for (int i = 0; i < 6; i++)
        sum += a[i];
    CHECK_EQ(sum, 21);
}

int main()
{
    return tf::runAll("Shuffle");
}