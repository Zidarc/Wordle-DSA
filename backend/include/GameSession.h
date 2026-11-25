#ifndef WORDLE_DSA_GAME_SESSION_H
#define WORDLE_DSA_GAME_SESSION_H

// One game session: coordinates Board (2D array), LetterTracker (array of 26 + jagged rows),
// Wordle (secret word BST + 2-pass feedback), Stack (guess history),
// Queue (shuffled hint positions), and DoublyLinkedList (game replay).

#include "Board.h"
#include "Dictionary.h"
#include "Difficulty.h"
#include "LetterTracker.h"
#include "Score.h"
#include "Wordle.h"
#include "ds/DoublyLinkedList.h"
#include "ds/Queue.h"
#include "ds/SafeArray.h"
#include "ds/Stack.h"

#include <string>

using namespace std;

enum class GameStatus
{
    Playing = 0,
    Won = 1,
    Lost = 2
};

inline const char* toString(GameStatus status)
{
    switch (status)
    {
    case GameStatus::Won:
        return "won";
    case GameStatus::Lost:
        return "lost";
    case GameStatus::Playing:
    default:
        return "playing";
    }
}

inline ostream& operator<<(ostream& os, GameStatus s)
{
    return os << toString(s);
}

struct HintResult
{
    bool available = false;
    int position = -1; // 0..4
    char letter = '\0';
    int hintsUsed = 0;
    int penalty = 0;
    string message;
};

struct GuessResult
{
    bool accepted = false;
    string errorMessage;
    GuessFeedback feedback;
    LookupStats lookupStats;
    LookupMethod methodUsed = LookupMethod::Hash; // the lookup that decided "in the word list?"
    int attemptsLeft = 0;
    int attemptsUsed = 0;
    GameStatus status = GameStatus::Playing;
    int score = 0;
};

class GameSession
{
public:
    GameSession();
    GameSession(const string& id, const string& secretWord,
                Difficulty difficulty = Difficulty::Normal, unsigned int hintSeed = 0);

    const string& id() const { return id_; }
    Difficulty difficulty() const { return difficulty_; }
    GameStatus status() const { return status_; }
    bool isGameOver() const { return status_ != GameStatus::Playing; }
    int maxAttempts() const { return maxAttempts_; }
    int attemptsUsed() const { return board_.filledRows(); }
    int attemptsLeft() const { return maxAttempts_ - board_.filledRows(); }
    int hintsUsed() const { return hintsUsed_; }
    // Has the hint for this position been revealed? (false if out of range)
    bool hintRevealedAt(int pos) const { return pos >= 0 && pos < 5 && hintRevealed_[pos]; }
    int score() const { return score_; }

    // A won game goes on the leaderboard once. The server checks this before
    // saving, so the same win cannot be posted again and again.
    bool scoreSaved() const { return scoreSaved_; }
    void markScoreSaved() { scoreSaved_ = true; }
    const string& secretWord() const { return wordle_.secret(); }

    const Board& board() const { return board_; }
    const LetterTracker& tracker() const { return tracker_; }
    const Stack<string>& history() const { return history_; }
    const DoublyLinkedList<GuessFeedback>& replay() const { return replay_; }

    // Validates the guess with the chosen lookup method (hash table by
    // default), then colours it. All six methods still run for the stats.
    GuessResult makeGuess(const string& rawGuess, const Dictionary& dictionary,
                          LookupMethod method = LookupMethod::Hash);
    HintResult useHint();

private:
    string id_;
    Difficulty difficulty_;
    int maxAttempts_;
    Wordle wordle_;
    Board board_;
    LetterTracker tracker_;
    Stack<string> history_;
    Queue<int> hintQueue_;
    DoublyLinkedList<GuessFeedback> replay_;
    int hintsUsed_;
    bool hintRevealed_[5];
    GameStatus status_;
    int score_;
    bool scoreSaved_;

    void initHintQueue(unsigned int seed);
};

#endif // WORDLE_DSA_GAME_SESSION_H
