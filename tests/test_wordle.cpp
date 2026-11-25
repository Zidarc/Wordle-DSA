#include "TestFramework.h"
#include "Board.h"
#include "Wordle.h"

#include <stdexcept>
#include <string>

// Feedback as a string: G = correct, Y = present, X = absent.
static std::string pattern(const GuessFeedback& feedback)
{
    std::string result;
    for (int i = 0; i < GuessFeedback::kLength; ++i)
    {
        switch (feedback.states[i])
        {
        case LetterState::Correct:
            result += 'G';
            break;
        case LetterState::Present:
            result += 'Y';
            break;
        default:
            result += 'X';
            break;
        }
    }
    return result;
}

static std::string colour(const char* secret, const char* guess)
{
    return pattern(Wordle(secret).evaluate(guess));
}

// ===========================================================================
// Construction
// ===========================================================================

TEST_CASE(wordleNormalisesTheSecret)
{
    Wordle game(" crane ");
    CHECK_EQ(game.secret(), std::string("CRANE"));
}

TEST_CASE(wordleRejectsBadSecret)
{
    CHECK_THROWS(Wordle("CRAN"));
    CHECK_THROWS(Wordle("CRANES"));
    CHECK_THROWS(Wordle("CR4NE"));
    CHECK_THROWS(Wordle(""));
}

TEST_CASE(wordleRejectsBadGuess)
{
    Wordle game("CRANE");
    CHECK_THROWS(game.evaluate("CRAN"));
    CHECK_THROWS(game.evaluate("CRANES"));
    CHECK_THROWS(game.evaluate("12345"));
}

// ===========================================================================
// Secret-letter BST
// ===========================================================================

TEST_CASE(wordleLetterBstHoldsDistinctLetters)
{
    Wordle crane("CRANE");
    CHECK_EQ(crane.distinctLetters(), 5);
    for (char c : std::string("CRANE"))
        CHECK(crane.containsLetter(c));
    CHECK(crane.containsLetter('n'));
    CHECK(!crane.containsLetter('Z'));
    CHECK(!crane.containsLetter('S'));

    Wordle apple("APPLE");
    CHECK_EQ(apple.distinctLetters(), 4); // A P L E
}

TEST_CASE(wordleLetterBstIsShallow)
{
    // Sorted, then inserted median-first, so 5 letters give a height of 3.
    CHECK_EQ(Wordle("CRANE").letterTreeHeight(), 3);
    CHECK_EQ(Wordle("ABCDE").letterTreeHeight(), 3);
    CHECK_EQ(Wordle("EDCBA").letterTreeHeight(), 3);
    CHECK(Wordle("APPLE").letterTreeHeight() <= 3);
    CHECK(Wordle("AAAAA").letterTreeHeight() == 1);
}

TEST_CASE(wordleLetterLookupCountsComparisons)
{
    Wordle game("CRANE");
    int comparisons = 0;
    CHECK(game.containsLetter('A', comparisons));
    // The BST counts up to 2 comparisons per node (< then >), and the tree has height 3.
    CHECK(comparisons >= 1);
    CHECK(comparisons <= 2 * game.letterTreeHeight());

    comparisons = 0;
    CHECK(game.containsLetter('E', comparisons)); // E is the root
    CHECK_EQ(comparisons, 2);
}

// ===========================================================================
// Two-pass comparison
// ===========================================================================

TEST_CASE(wordleAllGreenIsAWin)
{
    GuessFeedback fb = Wordle("CRANE").evaluate("crane");
    CHECK_EQ(pattern(fb), std::string("GGGGG"));
    CHECK(fb.isWin());
    CHECK_EQ(fb.guess, std::string("CRANE"));
}

TEST_CASE(wordleAllGray)
{
    GuessFeedback fb = Wordle("CRANE").evaluate("MOIST");
    CHECK_EQ(pattern(fb), std::string("XXXXX"));
    CHECK(!fb.isWin());
}

TEST_CASE(wordleMisplacedLetters)
{
    CHECK_EQ(colour("CRANE", "NACRE"), std::string("YYYYG"));
    CHECK_EQ(colour("CRANE", "SLATE"), std::string("XXGXG"));
    CHECK_EQ(colour("CRANE", "TRACE"), std::string("XGGYG"));
}

TEST_CASE(wordleRepeatedLettersInGuess)
{
    // The README examples.
    CHECK_EQ(colour("APPLE", "PAPER"), std::string("YYGYX"));
    CHECK_EQ(colour("APPLE", "PUPPY"), std::string("YXGXX"));

    // Only one E in the secret: the green E uses it, the other is gray.
    CHECK_EQ(colour("CRANE", "EERIE"), std::string("XXYXG"));
    // A letter not in the secret at all, repeated.
    CHECK_EQ(colour("CRANE", "LLAMA"), std::string("XXGXX"));
}

TEST_CASE(wordleGreenTakesPriorityOverEarlierYellow)
{
    // The second L is green; the first L must be gray because the secret has one L.
    CHECK_EQ(colour("WORLD", "LOLLY"), std::string("XGXGX"));
}

TEST_CASE(wordleRepeatedLettersInSecret)
{
    // Secret has two E's.
    CHECK_EQ(colour("GEESE", "EERIE"), std::string("YGXXG"));
    CHECK_EQ(colour("GEESE", "EVENT"), std::string("YXGXX"));
    // Secret has two P's; guess has one, misplaced.
    CHECK_EQ(colour("APPLE", "SPORT"), std::string("XGXXX"));
    CHECK_EQ(colour("APPLE", "PLANE"), std::string("YYYXG"));
}

TEST_CASE(wordleFeedbackReportsBstComparisons)
{
    GuessFeedback fb = Wordle("CRANE").evaluate("SLATE");
    CHECK(fb.bstComparisons > 0);

    GuessFeedback win = Wordle("CRANE").evaluate("CRANE");
    CHECK_EQ(win.bstComparisons, 0); // all green: pass 2 never asks the BST
}

TEST_CASE(wordleFullGameLossOnHardBoard)
{
    // Hard mode: 5 attempts, none correct -> the board fills and the game is lost.
    Wordle game("CRANE");
    Board board(5);
    const char* guesses[] = {"SLATE", "MOIST", "PLUMB", "GHOST", "TRACE"};
    bool won = false;
    for (const char* guess : guesses)
    {
        GuessFeedback fb = game.evaluate(guess);
        board.addRow(fb.guess, fb.states);
        won = won || fb.isWin();
    }
    CHECK(!won);
    CHECK(board.isFull());
    CHECK(board.stateAt(4, 1) == LetterState::Correct); // TRACE: R is green
}

int main()
{
    return tf::runAll("Wordle");
}
