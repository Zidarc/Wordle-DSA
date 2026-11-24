#ifndef WORDLE_DSA_SERVER_H
#define WORDLE_DSA_SERVER_H

// C++ REST API Server for Wordle-DSA.
//
// Manages:
//   - active game sessions stored in HashTable<std::string, GameSession>
//   - dictionary validation and 5-way lookup stats
//   - secret word selection via WordPicker
//   - persistent scores and top-10 leaderboard
//   - HTTP REST API routes and CORS headers

#include "Dictionary.h"
#include "GameSession.h"
#include "Leaderboard.h"
#include "WordPicker.h"
#include "ds/ArrayQueue.h"
#include "ds/HashTable.h"

#include <memory>
#include <string>

namespace httplib
{
class Server;
}

// One finished game, kept in the recent-games ring buffer.
struct RecentGame
{
    std::string gameId;
    std::string secretWord;
    std::string difficulty;
    std::string result; // "won" / "lost"
    int attempts = 0;
    int score = 0;
};

class Server
{
public:
    explicit Server(const std::string& wordsPath = "data/words.txt",
                    const std::string& scoresPath = "data/scores.txt");
    ~Server();

    // Initializes routes and starts listening on host:port. Blocks until stop() is called.
    bool listen(const std::string& host = "0.0.0.0", int port = 8080);

    // Stops the running HTTP server.
    void stop();

    const Dictionary& dictionary() const { return dictionary_; }
    const Leaderboard& leaderboard() const { return leaderboard_; }

private:
    Dictionary dictionary_;
    std::unique_ptr<WordPicker> picker_;
    Leaderboard leaderboard_;
    HashTable<std::string, GameSession> sessions_;
    std::unique_ptr<httplib::Server> http_;
    int gameCounter_ = 0;

    // The last kRecentGames finished games, oldest first. A bounded circular
    // ArrayQueue: when it is full, the oldest game is dropped (O(1)).
    static constexpr std::size_t kRecentGames = 10;
    ArrayQueue<RecentGame> recentGames_{kRecentGames, true};

    void setupRoutes();
    void setupDsaRoutes(); // solver, Sorting Lab and tree routes
    std::string generateGameId();
};

#endif // WORDLE_DSA_SERVER_H
