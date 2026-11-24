#include "Server.h"

#include "httplib.h"
#include "json.hpp"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>

using json = nlohmann::json;

namespace
{

void setCors(httplib::Response& res)
{
    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type");
}

std::string currentDate()
{
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm;
#if defined(_WIN32)
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d", &tm);
    return std::string(buf);
}

} // namespace

Server::Server(const std::string& wordsPath, const std::string& scoresPath)
    : leaderboard_(scoresPath),
      http_(std::make_unique<httplib::Server>()),
      gameCounter_(0)
{
    try
    {
        dictionary_.loadFromFile(wordsPath);
        std::cout << "[Server] Loaded dictionary with " << dictionary_.size() << " words.\n";
    }
    catch (const std::exception& e)
    {
        std::cerr << "[Server] Warning: failed to load dictionary: " << e.what() << "\n";
    }

    if (!dictionary_.empty())
        picker_ = std::make_unique<WordPicker>(dictionary_.sortedWords());

    setupRoutes();
}

Server::~Server() = default;

std::string Server::generateGameId()
{
    ++gameCounter_;
    auto now = std::chrono::steady_clock::now().time_since_epoch().count();
    return "game-" + std::to_string(gameCounter_) + "-" + std::to_string(now % 1000000);
}

void Server::setupRoutes()
{
    // Preflight CORS
    http_->Options(".*", [](const httplib::Request&, httplib::Response& res) {
        setCors(res);
        res.status = 204;
    });

    // POST /api/game/new
    http_->Post("/api/game/new", [this](const httplib::Request& req, httplib::Response& res) {
        setCors(res);
        if (!picker_)
        {
            res.status = 500;
            res.set_content(json{{"error", "Dictionary not loaded"}}.dump(), "application/json");
            return;
        }

        std::string diffStr = "normal";
        if (!req.body.empty())
        {
            try
            {
                auto body = json::parse(req.body);
                if (body.contains("difficulty") && body["difficulty"].is_string())
                    diffStr = body["difficulty"];
            }
            catch (...)
            {
            }
        }

        Difficulty diff = difficultyFromString(diffStr);
        std::string gameId = generateGameId();
        std::string secret = picker_->nextWord();

        GameSession session(gameId, secret, diff);
        sessions_.insert(gameId, session);

        json reply = {
            {"gameId", gameId},
            {"difficulty", toString(diff)},
            {"maxAttempts", session.maxAttempts()},
            {"wordLength", 5}
        };

        res.status = 200;
        res.set_content(reply.dump(), "application/json");
    });

    // POST /api/game/guess
    http_->Post("/api/game/guess", [this](const httplib::Request& req, httplib::Response& res) {
        setCors(res);
        std::string gameId, guess;
        try
        {
            auto body = json::parse(req.body);
            gameId = body.value("gameId", "");
            guess = body.value("guess", "");
        }
        catch (...)
        {
            res.status = 400;
            res.set_content(json{{"error", "Invalid JSON body"}}.dump(), "application/json");
            return;
        }

        GameSession* session = sessions_.find(gameId);
        if (!session)
        {
            res.status = 404;
            res.set_content(json{{"error", "Game session not found"}}.dump(), "application/json");
            return;
        }

        GuessResult result = session->makeGuess(guess, dictionary_);

        json reply;
        reply["guess"] = Dictionary::normalize(guess);
        reply["valid"] = result.accepted;
        reply["attemptsLeft"] = result.attemptsLeft;
        reply["attemptsUsed"] = result.attemptsUsed;
        reply["status"] = toString(result.status);
        reply["score"] = result.score;

        // Lookup stats for the DSA Insights panel
        reply["lookupStats"] = {
            {"linear", result.lookupStats.linear.comparisons},
            {"binary", result.lookupStats.binary.comparisons},
            {"interpolation", result.lookupStats.interpolation.comparisons},
            {"avl", result.lookupStats.avl.comparisons},
            {"hash", result.lookupStats.hash.comparisons}
        };

        if (!result.accepted)
        {
            reply["error"] = result.errorMessage;
            res.status = 400;
            res.set_content(reply.dump(), "application/json");
            return;
        }

        // Feedback array
        json feedback = json::array();
        for (int i = 0; i < 5; ++i)
            feedback.push_back(toString(result.feedback.states[i]));
        reply["feedback"] = feedback;

        // Keyboard jagged rows
        json keyboard = json::array();
        const LetterTracker& tracker = session->tracker();
        for (int r = 0; r < tracker.keyboardRowCount(); ++r)
        {
            json row = json::array();
            for (int c = 0; c < tracker.keyboardRowLength(r); ++c)
                row.push_back(std::string(1, tracker.keyAt(r, c)));
            keyboard.push_back(row);
        }
        reply["keyboard"] = keyboard;

        // Letter status map (A-Z)
        json letterStatus = json::object();
        for (char c = 'A'; c <= 'Z'; ++c)
        {
            LetterState state = tracker.statusOf(c);
            if (state != LetterState::Unused)
                letterStatus[std::string(1, c)] = toString(state);
        }
        reply["letterStatus"] = letterStatus;

        // Secret word revealed only after game ends
        if (session->isGameOver())
            reply["secretWord"] = session->secretWord();
        else
            reply["secretWord"] = "";

        res.status = 200;
        res.set_content(reply.dump(), "application/json");
    });

    // POST /api/game/hint
    http_->Post("/api/game/hint", [this](const httplib::Request& req, httplib::Response& res) {
        setCors(res);
        std::string gameId;
        try
        {
            auto body = json::parse(req.body);
            gameId = body.value("gameId", "");
        }
        catch (...)
        {
            res.status = 400;
            res.set_content(json{{"error", "Invalid JSON body"}}.dump(), "application/json");
            return;
        }

        GameSession* session = sessions_.find(gameId);
        if (!session)
        {
            res.status = 404;
            res.set_content(json{{"error", "Game session not found"}}.dump(), "application/json");
            return;
        }

        HintResult hint = session->useHint();
        if (!hint.available)
        {
            res.status = 400;
            res.set_content(json{{"error", hint.message}}.dump(), "application/json");
            return;
        }

        json reply = {
            {"position", hint.position},
            {"letter", std::string(1, hint.letter)},
            {"hintsUsed", hint.hintsUsed},
            {"penalty", hint.penalty},
            {"message", hint.message}
        };

        res.status = 200;
        res.set_content(reply.dump(), "application/json");
    });

    // GET /api/game/history?gameId=...
    http_->Get("/api/game/history", [this](const httplib::Request& req, httplib::Response& res) {
        setCors(res);
        std::string gameId = req.get_param_value("gameId");
        GameSession* session = sessions_.find(gameId);
        if (!session)
        {
            res.status = 404;
            res.set_content(json{{"error", "Game session not found"}}.dump(), "application/json");
            return;
        }

        json history = json::array();
        for (const std::string& word : session->history())
            history.push_back(word);

        json reply = {
            {"gameId", gameId},
            {"history", history}
        };

        res.status = 200;
        res.set_content(reply.dump(), "application/json");
    });

    // GET /api/game/replay?gameId=...
    http_->Get("/api/game/replay", [this](const httplib::Request& req, httplib::Response& res) {
        setCors(res);
        std::string gameId = req.get_param_value("gameId");
        GameSession* session = sessions_.find(gameId);
        if (!session)
        {
            res.status = 404;
            res.set_content(json{{"error", "Game session not found"}}.dump(), "application/json");
            return;
        }

        if (!session->isGameOver())
        {
            res.status = 400;
            res.set_content(json{{"error", "Replay is only available after game is over"}}.dump(), "application/json");
            return;
        }

        json steps = json::array();
        int stepNum = 1;
        for (const GuessFeedback& fb : session->replay())
        {
            json feedbackStates = json::array();
            for (int i = 0; i < 5; ++i)
                feedbackStates.push_back(toString(fb.states[i]));

            steps.push_back({
                {"step", stepNum++},
                {"guess", fb.guess},
                {"feedback", feedbackStates}
            });
        }

        json reply = {
            {"gameId", gameId},
            {"secretWord", session->secretWord()},
            {"status", toString(session->status())},
            {"score", session->score()},
            {"steps", steps}
        };

        res.status = 200;
        res.set_content(reply.dump(), "application/json");
    });

    // GET /api/leaderboard?search=...
    http_->Get("/api/leaderboard", [this](const httplib::Request& req, httplib::Response& res) {
        setCors(res);
        std::string query = req.get_param_value("search");
        SafeArray<LeaderboardEntry> top = leaderboard_.topScores(query);

        json list = json::array();
        for (std::size_t i = 0; i < top.size(); ++i)
        {
            list.push_back({
                {"rank", static_cast<int>(i + 1)},
                {"name", top[i].name},
                {"score", top[i].score},
                {"difficulty", top[i].difficulty},
                {"date", top[i].date}
            });
        }

        res.status = 200;
        res.set_content(list.dump(), "application/json");
    });

    // POST /api/leaderboard
    http_->Post("/api/leaderboard", [this](const httplib::Request& req, httplib::Response& res) {
        setCors(res);
        std::string gameId, name;
        try
        {
            auto body = json::parse(req.body);
            gameId = body.value("gameId", "");
            name = body.value("name", "Anonymous");
        }
        catch (...)
        {
            res.status = 400;
            res.set_content(json{{"error", "Invalid JSON body"}}.dump(), "application/json");
            return;
        }

        GameSession* session = sessions_.find(gameId);
        if (!session)
        {
            res.status = 404;
            res.set_content(json{{"error", "Game session not found"}}.dump(), "application/json");
            return;
        }

        if (session->status() != GameStatus::Won)
        {
            res.status = 400;
            res.set_content(json{{"error", "Can only post score for won games"}}.dump(), "application/json");
            return;
        }

        std::string today = currentDate();
        leaderboard_.addScore(name, session->score(), toString(session->difficulty()), today);

        res.status = 200;
        res.set_content(json{
            {"saved", true},
            {"name", name},
            {"score", session->score()},
            {"difficulty", toString(session->difficulty())},
            {"date", today}
        }.dump(), "application/json");
    });

    // GET /api/dsa/stats
    http_->Get("/api/dsa/stats", [this](const httplib::Request&, httplib::Response& res) {
        setCors(res);
        DictionaryStats s = dictionary_.stats();
        json reply = {
            {"wordCount", s.wordCount},
            {"avlHeight", s.avlHeight},
            {"hashSize", s.hashSize},
            {"hashBuckets", s.hashBuckets},
            {"hashUsedBuckets", s.hashUsedBuckets},
            {"hashLongestChain", s.hashLongestChain},
            {"hashRehashCount", s.hashRehashCount},
            {"hashLoadFactor", s.hashLoadFactor}
        };

        res.status = 200;
        res.set_content(reply.dump(), "application/json");
    });
}

bool Server::listen(const std::string& host, int port)
{
    std::cout << "[Server] Wordle-DSA HTTP Server listening on http://" << host << ":" << port << "\n";
    return http_->listen(host, port);
}

void Server::stop()
{
    if (http_)
        http_->stop();
}
