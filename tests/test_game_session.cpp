#include "TestFramework.h"
#include "GameSession.h"

#include <sstream>

#ifndef WORDLE_DATA_DIR
#define WORDLE_DATA_DIR "data"
#endif

static Dictionary makeDictionary()
{
    std::istringstream input("slate\nCRANE\napple\nbrave\nplumb\npuppy\npaper\n");
    Dictionary dict;
    dict.loadFromStream(input);
    return dict;
}

TEST_CASE(gameSessionInitialState)
{
    GameSession session("game-1", "CRANE", Difficulty::Normal);
    CHECK_EQ(session.id(), std::string("game-1"));
    CHECK_EQ(session.difficulty(), Difficulty::Normal);
    CHECK_EQ(session.maxAttempts(), 6);
    CHECK_EQ(session.attemptsUsed(), 0);
    CHECK_EQ(session.attemptsLeft(), 6);
    CHECK_EQ(session.status(), GameStatus::Playing);
    CHECK(!session.isGameOver());
    CHECK_EQ(session.hintsUsed(), 0);
}

TEST_CASE(gameSessionRejectsNotInWordList)
{
    Dictionary dict = makeDictionary();
    GameSession session("game-1", "CRANE", Difficulty::Normal);

    GuessResult res = session.makeGuess("zzzzz", dict);
    CHECK(!res.accepted);
    CHECK_EQ(res.errorMessage, std::string("Not in word list"));
    CHECK_EQ(session.attemptsUsed(), 0); // Attempt not consumed
}

TEST_CASE(gameSessionUsesTheChosenLookupMethod)
{
    Dictionary dict = makeDictionary();
    const LookupMethod methods[] = {LookupMethod::Linear, LookupMethod::Binary,
                                    LookupMethod::Interpolation, LookupMethod::Avl,
                                    LookupMethod::BTree, LookupMethod::Hash};
    for (LookupMethod m : methods)
    {
        GameSession session("game-m", "CRANE", Difficulty::Easy);
        GuessResult bad = session.makeGuess("zzzzz", dict, m);
        CHECK(!bad.accepted);
        CHECK(bad.methodUsed == m);
        GuessResult good = session.makeGuess("slate", dict, m);
        CHECK(good.accepted);
        CHECK(good.lookupStats.get(m).found);
        CHECK(good.lookupStats.get(m).comparisons >= 1);
    }
}

TEST_CASE(gameSessionWinEvaluation)
{
    Dictionary dict = makeDictionary();
    GameSession session("game-1", "CRANE", Difficulty::Normal);

    GuessResult res = session.makeGuess("crane", dict);
    CHECK(res.accepted);
    CHECK(res.feedback.isWin());
    CHECK_EQ(session.status(), GameStatus::Won);
    CHECK(session.isGameOver());
    CHECK(session.score() > 0);
    CHECK_EQ(session.attemptsUsed(), 1);
    CHECK_EQ(session.history().size(), 1u);
    CHECK_EQ(session.history().peek(), std::string("CRANE"));
}

TEST_CASE(gameSessionLossWhenFull)
{
    Dictionary dict = makeDictionary();
    GameSession session("game-1", "CRANE", Difficulty::Hard); // 5 attempts

    session.makeGuess("slate", dict);
    session.makeGuess("apple", dict);
    session.makeGuess("brave", dict);
    session.makeGuess("plumb", dict);
    GuessResult res = session.makeGuess("puppy", dict);

    CHECK(res.accepted);
    CHECK(!res.feedback.isWin());
    CHECK_EQ(session.status(), GameStatus::Lost);
    CHECK(session.isGameOver());
    CHECK_EQ(session.score(), 0);
    CHECK_EQ(session.attemptsUsed(), 5);
}

TEST_CASE(gameSessionHintRevealsLetterAndPenalizes)
{
    GameSession session("game-1", "CRANE", Difficulty::Normal, 42u);

    HintResult hint1 = session.useHint();
    CHECK(hint1.available);
    CHECK(hint1.position >= 0 && hint1.position < 5);
    CHECK_EQ(hint1.letter, session.secretWord()[hint1.position]);
    CHECK_EQ(hint1.hintsUsed, 1);
    CHECK_EQ(hint1.penalty, 50);

    HintResult hint2 = session.useHint();
    CHECK(hint2.available);
    CHECK_EQ(hint2.hintsUsed, 2);
    CHECK_EQ(hint2.penalty, 100);
}

TEST_CASE(gameSessionRemembersRevealedPositions)
{
    GameSession session("game-1", "CRANE", Difficulty::Normal, 7u);
    for (int p = 0; p < 5; ++p)
        CHECK(!session.hintRevealedAt(p));

    HintResult hint = session.useHint();
    CHECK(hint.available);
    for (int p = 0; p < 5; ++p)
        CHECK_EQ(session.hintRevealedAt(p), p == hint.position);
    CHECK(!session.hintRevealedAt(-1));
    CHECK(!session.hintRevealedAt(5));
}

TEST_CASE(gameSessionHardModeEnforcesHintReuse)
{
    Dictionary dict = makeDictionary();
    GameSession session("game-1", "CRANE", Difficulty::Hard, 100u);

    HintResult hint = session.useHint();
    CHECK(hint.available);

    // Guess "SLATE" vs secret "CRANE"
    // If the revealed hint was at position 0 ('C'), SLATE doesn't have 'C' at pos 0!
    // Let's craft a guess that misses the hint
    std::string badGuess = "SLATE";
    if (badGuess[hint.position] == hint.letter)
        badGuess = "PLUMB"; // switch to one that doesn't match

    GuessResult res = session.makeGuess(badGuess, dict);
    CHECK(!res.accepted);
    CHECK(res.errorMessage.find("Hard mode: revealed hint") != std::string::npos);
    CHECK_EQ(session.attemptsUsed(), 0); // Not consumed!
}

int main()
{
    return tf::runAll("GameSession");
}
