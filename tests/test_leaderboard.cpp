#include "TestFramework.h"
#include "Leaderboard.h"

#include <cstdio>
#include <fstream>
#include <string>

#ifndef WORDLE_DATA_DIR
#define WORDLE_DATA_DIR "data"
#endif

static const std::string kTestScoresFile = std::string(WORDLE_DATA_DIR) + "/test_scores.txt";

static void cleanTestFile()
{
    std::remove(kTestScoresFile.c_str());
}

TEST_CASE(leaderboardHandlesMissingFile)
{
    cleanTestFile();
    Leaderboard lb("does/not/exist/scores.txt");
    SafeArray<LeaderboardEntry> top = lb.topScores();
    CHECK(top.empty());
}

TEST_CASE(leaderboardSavesAndLoadsScores)
{
    cleanTestFile();
    Leaderboard lb(kTestScoresFile);

    CHECK(lb.addScore("Alice", 500, "normal", "2026-11-19"));
    CHECK(lb.addScore("Bob", 650, "hard", "2026-11-19"));
    CHECK(lb.addScore("Charlie", 350, "easy", "2026-11-19"));

    SafeArray<LeaderboardEntry> top = lb.topScores();
    CHECK_EQ(top.size(), 3u);

    // Sorted descending by score (Bob 650, Alice 500, Charlie 350)
    CHECK_EQ(top[0].name, std::string("Bob"));
    CHECK_EQ(top[0].score, 650);
    CHECK_EQ(top[1].name, std::string("Alice"));
    CHECK_EQ(top[1].score, 500);
    CHECK_EQ(top[2].name, std::string("Charlie"));
    CHECK_EQ(top[2].score, 350);

    cleanTestFile();
}

TEST_CASE(leaderboardTopKLimitsToTen)
{
    cleanTestFile();
    Leaderboard lb(kTestScoresFile);

    // Insert 15 scores
    for (int i = 1; i <= 15; ++i)
    {
        lb.addScore("Player" + std::to_string(i), i * 50, "normal", "2026-11-19");
    }

    SafeArray<LeaderboardEntry> top = lb.topScores();
    CHECK_EQ(top.size(), 10u);

    // Highest should be Player15 (750)
    CHECK_EQ(top[0].name, std::string("Player15"));
    CHECK_EQ(top[0].score, 750);

    // 10th should be Player6 (300)
    CHECK_EQ(top[9].name, std::string("Player6"));
    CHECK_EQ(top[9].score, 300);

    cleanTestFile();
}

TEST_CASE(leaderboardKmpSearchFiltersByName)
{
    cleanTestFile();
    Leaderboard lb(kTestScoresFile);

    lb.addScore("Ali Hussain", 700, "normal", "2026-11-19");
    lb.addScore("Muhammad Abbas", 650, "hard", "2026-11-19");
    lb.addScore("Ali Asghar", 600, "normal", "2026-11-19");
    lb.addScore("Zidarc", 500, "easy", "2026-11-19");

    SafeArray<LeaderboardEntry> searchAli = lb.topScores("ali");
    CHECK_EQ(searchAli.size(), 2u);
    CHECK_EQ(searchAli[0].name, std::string("Ali Hussain"));
    CHECK_EQ(searchAli[1].name, std::string("Ali Asghar"));

    SafeArray<LeaderboardEntry> searchAbbas = lb.topScores("Abbas");
    CHECK_EQ(searchAbbas.size(), 1u);
    CHECK_EQ(searchAbbas[0].name, std::string("Muhammad Abbas"));

    SafeArray<LeaderboardEntry> searchNone = lb.topScores("xyz");
    CHECK_EQ(searchNone.size(), 0u);

    cleanTestFile();
}

int main()
{
    cleanTestFile();
    int res = tf::runAll("Leaderboard");
    cleanTestFile();
    return res;
}
