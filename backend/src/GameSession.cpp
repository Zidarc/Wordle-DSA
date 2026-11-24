#include "GameSession.h"

#include "algorithms/Shuffle.h"

#include <random>
#include <stdexcept>

GameSession::GameSession()
    : id_(""),
      difficulty_(Difficulty::Normal),
      maxAttempts_(6),
      wordle_("CRANE"),
      board_(6, 5),
      hintsUsed_(0),
      status_(GameStatus::Playing),
      score_(0)
{
    for (int i = 0; i < 5; ++i)
        hintRevealed_[i] = false;
    initHintQueue(0);
}

GameSession::GameSession(const std::string& id, const std::string& secretWord,
                         Difficulty difficulty, unsigned int hintSeed)
    : id_(id),
      difficulty_(difficulty),
      maxAttempts_(::maxAttempts(difficulty)),
      wordle_(secretWord),
      board_(::maxAttempts(difficulty), 5),
      hintsUsed_(0),
      status_(GameStatus::Playing),
      score_(0)
{
    for (int i = 0; i < 5; ++i)
        hintRevealed_[i] = false;
    initHintQueue(hintSeed);
}

void GameSession::initHintQueue(unsigned int seed)
{
    hintQueue_.clear();
    SafeArray<int> positions(5);
    for (int i = 0; i < 5; ++i)
        positions[i] = i;

    if (seed != 0)
        shuffleArray(positions, 5, seed);
    else
        shuffleArray(positions, 5);

    for (std::size_t i = 0; i < positions.size(); ++i)
        hintQueue_.enqueue(positions[i]);
}

HintResult GameSession::useHint()
{
    HintResult res;
    res.hintsUsed = hintsUsed_;
    res.penalty = hintsUsed_ * 50;

    if (status_ != GameStatus::Playing)
    {
        res.available = false;
        res.message = "Game is already over";
        return res;
    }

    while (!hintQueue_.empty())
    {
        int pos = hintQueue_.dequeue();
        if (hintRevealed_[pos])
            continue;

        // Check if player has already placed this letter correctly in any row.
        bool alreadySolved = false;
        for (int r = 0; r < board_.filledRows(); ++r)
        {
            if (board_.stateAt(r, pos) == LetterState::Correct)
            {
                alreadySolved = true;
                break;
            }
        }

        if (!alreadySolved)
        {
            hintRevealed_[pos] = true;
            ++hintsUsed_;
            res.available = true;
            res.position = pos;
            res.letter = wordle_.secret()[pos];
            res.hintsUsed = hintsUsed_;
            res.penalty = hintsUsed_ * 50;
            res.message = "Hint revealed";
            return res;
        }
    }

    res.available = false;
    res.message = "No more hints available";
    return res;
}

GuessResult GameSession::makeGuess(const std::string& rawGuess, const Dictionary& dictionary,
                                   LookupMethod method)
{
    GuessResult res;
    res.methodUsed = method;
    res.attemptsLeft = attemptsLeft();
    res.attemptsUsed = attemptsUsed();
    res.status = status_;
    res.score = score_;

    if (status_ != GameStatus::Playing)
    {
        res.accepted = false;
        res.errorMessage = "Game is already over";
        return res;
    }

    std::string guess = Dictionary::normalize(rawGuess);
    if (!Dictionary::isWellFormed(guess))
    {
        res.accepted = false;
        res.errorMessage = "Guess must be a 5-letter word (A-Z)";
        return res;
    }

    res.lookupStats = dictionary.lookupAll(guess);

    // The player's chosen method decides. All six always agree (tested in
    // test_dictionary), so the choice changes the cost, never the answer.
    if (!res.lookupStats.get(method).found)
    {
        res.accepted = false;
        res.errorMessage = "Not in word list";
        return res;
    }

    // Hard mode constraint check
    if (requiresHintReuse(difficulty_))
    {
        for (int i = 0; i < 5; ++i)
        {
            if (hintRevealed_[i] && guess[i] != wordle_.secret()[i])
            {
                res.accepted = false;
                res.errorMessage = std::string("Hard mode: revealed hint '") +
                                   wordle_.secret()[i] + "' must be used at position " +
                                   std::to_string(i + 1);
                return res;
            }
        }
    }

    // Apply guess
    GuessFeedback fb = wordle_.evaluate(guess);
    board_.addRow(fb.guess, fb.states);
    tracker_.update(fb.guess, fb.states);
    history_.push(fb.guess);
    replay_.pushBack(fb);

    res.accepted = true;
    res.feedback = fb;
    res.attemptsUsed = board_.filledRows();
    res.attemptsLeft = maxAttempts_ - board_.filledRows();

    if (fb.isWin())
    {
        status_ = GameStatus::Won;
        score_ = Score::calculate(maxAttempts_, board_.filledRows(), hintsUsed_, difficulty_, true);
    }
    else if (board_.isFull())
    {
        status_ = GameStatus::Lost;
        score_ = 0;
    }

    res.status = status_;
    res.score = score_;
    return res;
}
