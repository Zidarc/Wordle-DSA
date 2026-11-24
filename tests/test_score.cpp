#include "TestFramework.h"
#include "Score.h"

TEST_CASE(scoreLostGameIsZero)
{
    CHECK_EQ(Score::calculate(6, 6, 0, Difficulty::Normal, false), 0);
    CHECK_EQ(Score::calculate(8, 8, 2, Difficulty::Easy, false), 0);
    CHECK_EQ(Score::calculate(5, 5, 1, Difficulty::Hard, false), 0);
}

TEST_CASE(scoreFormulaMatchesNormalDifficulty)
{
    // Normal mode (6 max attempts, 1.5x multiplier):
    // Solved on attempt 3 with 1 hint:
    // ((6 - 3 + 1) * 100 - 1 * 50) * 1.5 = (400 - 50) * 1.5 = 350 * 1.5 = 525
    CHECK_EQ(Score::calculate(6, 3, 1, Difficulty::Normal, true), 525);

    // Solved on attempt 1 with 0 hints:
    // ((6 - 1 + 1) * 100 - 0) * 1.5 = 600 * 1.5 = 900
    CHECK_EQ(Score::calculate(6, 1, 0, Difficulty::Normal, true), 900);
}

TEST_CASE(scoreFormulaMatchesEasyAndHard)
{
    // Easy mode (8 max attempts, 1.0x multiplier):
    // Solved on attempt 4 with 0 hints:
    // (8 - 4 + 1) * 100 * 1.0 = 500
    CHECK_EQ(Score::calculate(8, 4, 0, Difficulty::Easy, true), 500);

    // Hard mode (5 max attempts, 2.0x multiplier):
    // Solved on attempt 2 with 0 hints:
    // (5 - 2 + 1) * 100 * 2.0 = 400 * 2.0 = 800
    CHECK_EQ(Score::calculate(5, 2, 0, Difficulty::Hard, true), 800);
}

TEST_CASE(scoreNeverGoesBelowZero)
{
    // Max penalties: 10 hints used (500 pts penalty) on attempt 6 (100 base)
    // 100 - 500 = -400 -> clamped to 0
    CHECK_EQ(Score::calculate(6, 6, 10, Difficulty::Normal, true), 0);
}

int main()
{
    return tf::runAll("Score");
}
