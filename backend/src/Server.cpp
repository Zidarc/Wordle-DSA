#include "Server.h"

#include "Solver.h"
#include "SortingLab.h"

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

// One row per lookup method: answer, comparisons, time and Big-O.
// TIP: the frontend's DSA Insights table is drawn straight from this.
json lookupMethodsJson(const LookupStats& stats, LookupMethod used)
{
    json rows = json::array();
    for (int i = 0; i < Dictionary::kMethodCount; ++i)
    {
        const LookupMethodInfo& info = Dictionary::methodInfo(i);
        const LookupResult& r = stats.get(info.method);
        rows.push_back({{"key", info.key},
                        {"name", info.name},
                        {"structure", info.structure},
                        {"found", r.found},
                        {"comparisons", r.comparisons},
                        {"nanoseconds", r.nanoseconds},
                        {"timeBest", info.timeBest},
                        {"timeAvg", info.timeAvg},
                        {"timeWorst", info.timeWorst},
                        {"spaceStructure", info.spaceStructure},
                        {"spaceExtra", info.spaceExtra},
                        {"used", info.method == used}});
    }
    return rows;
}

// Rebuilds the solver state of a game by replaying its accepted guesses
// (the doubly linked replay list) in order, then its revealed hints.
// GUIDE 3.7: hints count as known greens, so the solver uses them too
void replayInto(Solver& solver, const GameSession& session)
{
    for (const GuessFeedback& fb : session.replay())
        solver.applyGuess(fb.guess, fb.states);
    for (int pos = 0; pos < Constraints::kLength; ++pos)
    {
        if (session.hintRevealedAt(pos))
            solver.applyHint(pos, session.secretWord()[pos]);
    }
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
    // GUIDE 3.2: new game route
    http_->Post("/api/game/new", [this](const httplib::Request& req, httplib::Response& res) {
        setCors(res);
        std::lock_guard<std::mutex> lock(mutex_);
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
    // GUIDE 3.3: guess route -> GameSession::makeGuess
    http_->Post("/api/game/guess", [this](const httplib::Request& req, httplib::Response& res) {
        setCors(res);
        std::lock_guard<std::mutex> lock(mutex_);
        std::string gameId, guess, methodName;
        try
        {
            auto body = json::parse(req.body);
            gameId = body.value("gameId", "");
            guess = body.value("guess", "");
            methodName = body.value("method", "hash");
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

        LookupMethod method = LookupMethod::Hash;
        if (!Dictionary::methodFromString(methodName, method))
        {
            res.status = 400;
            res.set_content(json{{"error", "Unknown lookup method '" + methodName + "'"}}.dump(),
                            "application/json");
            return;
        }

        GuessResult result = session->makeGuess(guess, dictionary_, method);

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
            {"btree", result.lookupStats.btree.comparisons},
            {"hash", result.lookupStats.hash.comparisons}
        };
        reply["btreeNodesVisited"] = result.lookupStats.btreeNodesVisited;
        reply["methodUsed"] = Dictionary::methodInfo(result.methodUsed).key;
        reply["lookupMethods"] = lookupMethodsJson(result.lookupStats, result.methodUsed);

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

        // This guess ended the game: remember it in the recent-games buffer.
        if (session->isGameOver())
        {
            RecentGame g;
            g.gameId = session->id();
            g.secretWord = session->secretWord();
            g.difficulty = toString(session->difficulty());
            g.result = toString(session->status());
            g.attempts = session->attemptsUsed();
            g.score = session->score();
            recentGames_.enqueue(g);
        }

        // How many dictionary words still fit all the feedback (solver list)
        Solver solver(dictionary_);
        replayInto(solver, *session);
        reply["possibleWords"] = solver.remaining();

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
        std::lock_guard<std::mutex> lock(mutex_);
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
        std::lock_guard<std::mutex> lock(mutex_);
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
        std::lock_guard<std::mutex> lock(mutex_);
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
        std::lock_guard<std::mutex> lock(mutex_);
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
        std::lock_guard<std::mutex> lock(mutex_);
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

        if (session->scoreSaved())
        {
            res.status = 409;
            res.set_content(json{{"error", "Score for this game is already saved"}}.dump(), "application/json");
            return;
        }

        std::string today = currentDate();
        name = Leaderboard::cleanName(name);
        if (!leaderboard_.addScore(name, session->score(), toString(session->difficulty()), today))
        {
            res.status = 500;
            res.set_content(json{{"error", "Could not write the scores file"}}.dump(), "application/json");
            return;
        }
        session->markScoreSaved();

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
        std::lock_guard<std::mutex> lock(mutex_);
        DictionaryStats s = dictionary_.stats();
        json reply = {
            {"wordCount", s.wordCount},
            {"avlHeight", s.avlHeight},
            {"hashSize", s.hashSize},
            {"hashBuckets", s.hashBuckets},
            {"hashUsedBuckets", s.hashUsedBuckets},
            {"hashLongestChain", s.hashLongestChain},
            {"hashRehashCount", s.hashRehashCount},
            {"hashLoadFactor", s.hashLoadFactor},
            {"avlLeaves", s.avlLeaves},
            {"avlBalanced", s.avlBalanced},
            {"btreeHeight", s.btreeHeight},
            {"btreeNodes", s.btreeNodes}
        };

        res.status = 200;
        res.set_content(reply.dump(), "application/json");
    });

    setupDsaRoutes();
}

// GUIDE 3.7: solver, lookup, trees, Sorting Lab, recent games
void Server::setupDsaRoutes()
{
    // GET /api/game/solver?gameId=...&count=5
    // Possible words, backtracking statistics (recursive and explicit-stack)
    // and the top suggestions ranked with quick sort.
    http_->Get("/api/game/solver", [this](const httplib::Request& req, httplib::Response& res) {
        setCors(res);
        std::lock_guard<std::mutex> lock(mutex_);
        std::string gameId = req.get_param_value("gameId");
        GameSession* session = sessions_.find(gameId);
        if (!session)
        {
            res.status = 404;
            res.set_content(json{{"error", "Game session not found"}}.dump(), "application/json");
            return;
        }

        std::size_t count = 5;
        if (req.has_param("count"))
        {
            try
            {
                count = static_cast<std::size_t>(std::stoul(req.get_param_value("count")));
            }
            catch (...)
            {
            }
        }

        Solver solver(dictionary_);
        replayInto(solver, *session);

        BacktrackStats rec;
        BacktrackStats iter;
        solver.generateCandidates(&rec);
        solver.generateCandidatesIterative(&iter);

        auto statsJson = [](const BacktrackStats& b) {
            return json{{"nodesVisited", b.nodesVisited},
                        {"pruned", b.pruned},
                        {"prefixChecks", b.prefixChecks},
                        {"maxDepth", b.maxDepth},
                        {"solutions", b.solutions}};
        };

        json suggestions = json::array();
        // Suggestions would give the answer away, so only after a few guesses
        // or once the game is over.
        if (session->isGameOver() || session->attemptsUsed() >= 2)
        {
            SafeArray<RankedWord> top = solver.suggestions(count);
            for (std::size_t i = 0; i < top.size(); ++i)
                suggestions.push_back({{"word", top[i].word}, {"score", top[i].score}});
        }

        json reply = {
            {"gameId", gameId},
            {"possibleWords", solver.remaining()},
            {"backtracking", statsJson(rec)},
            {"backtrackingIterative", statsJson(iter)},
            {"suggestions", suggestions}
        };
        res.status = 200;
        res.set_content(reply.dump(), "application/json");
    });

    // GET /api/dsa/lookup?word=CRANE&method=avl
    // Looks any word up with all six methods (no game needed): answer,
    // comparisons, average time in ns, and time / space complexity.
    http_->Get("/api/dsa/lookup", [this](const httplib::Request& req, httplib::Response& res) {
        setCors(res);
        std::lock_guard<std::mutex> lock(mutex_);
        std::string word = req.get_param_value("word");
        LookupMethod method = LookupMethod::Hash;
        if (req.has_param("method") && !Dictionary::methodFromString(req.get_param_value("method"), method))
        {
            res.status = 400;
            res.set_content(json{{"error", "Unknown lookup method"}}.dump(), "application/json");
            return;
        }
        if (!Dictionary::isWellFormed(word))
        {
            res.status = 400;
            res.set_content(json{{"error", "Word must be 5 letters A-Z"}}.dump(), "application/json");
            return;
        }

        LookupStats stats = dictionary_.lookupAll(word);
        json reply = {{"word", stats.word},
                      {"found", stats.get(method).found},
                      {"methodUsed", Dictionary::methodInfo(method).key},
                      {"wordCount", dictionary_.size()},
                      {"timingRepeats", Dictionary::kTimingRepeats},
                      {"methods", lookupMethodsJson(stats, method)}};
        res.status = 200;
        res.set_content(reply.dump(), "application/json");
    });

    // GET /api/games/recent
    // The last 10 finished games, most recent first (circular ArrayQueue).
    http_->Get("/api/games/recent", [this](const httplib::Request&, httplib::Response& res) {
        setCors(res);
        std::lock_guard<std::mutex> lock(mutex_);
        json list = json::array();
        for (std::size_t i = recentGames_.size(); i > 0; --i)
        {
            const RecentGame& g = recentGames_.at(i - 1);
            list.push_back({{"gameId", g.gameId},
                            {"secretWord", g.secretWord},
                            {"difficulty", g.difficulty},
                            {"result", g.result},
                            {"attempts", g.attempts},
                            {"score", g.score}});
        }
        res.status = 200;
        res.set_content(list.dump(), "application/json");
    });

    // GET /api/dsa/sort?limit=N
    // Sorting Lab: seven sorts on the same shuffled copy of the dictionary.
    http_->Get("/api/dsa/sort", [this](const httplib::Request& req, httplib::Response& res) {
        setCors(res);
        std::lock_guard<std::mutex> lock(mutex_);
        std::size_t limit = 0;
        if (req.has_param("limit"))
        {
            try
            {
                limit = static_cast<std::size_t>(std::stoul(req.get_param_value("limit")));
            }
            catch (...)
            {
            }
        }

        SortingLab lab(dictionary_.sortedWords(), 2513, limit);
        SafeArray<SortRun> runs = lab.runAll();
        json list = json::array();
        for (std::size_t i = 0; i < runs.size(); ++i)
        {
            list.push_back({{"algorithm", runs[i].algorithm},
                            {"complexity", runs[i].complexity},
                            {"comparisons", runs[i].comparisons},
                            {"swaps", runs[i].swaps},
                            {"microseconds", runs[i].microseconds},
                            {"sortedCorrectly", runs[i].sortedCorrectly}});
        }
        res.status = 200;
        res.set_content(json{{"inputSize", lab.inputSize()}, {"runs", list}}.dump(),
                        "application/json");
    });

    // GET /api/dsa/trees
    // Binary tree properties of the dictionary's AVL tree and the B-tree.
    http_->Get("/api/dsa/trees", [this](const httplib::Request&, httplib::Response& res) {
        setCors(res);
        std::lock_guard<std::mutex> lock(mutex_);
        TreeShape avl = dictionary_.avlShape();
        const BTree<std::string>& bt = dictionary_.btree();
        json reply = {
            {"avl", {{"nodes", avl.nodes},
                     {"leaves", avl.leaves},
                     {"internalNodes", avl.internalNodes},
                     {"singleChildNodes", avl.singleChildNodes},
                     {"height", avl.height},
                     {"minPossibleHeight", avl.minPossibleHeight},
                     {"full", avl.full},
                     {"complete", avl.complete},
                     {"perfect", avl.perfect}}},
            {"btree", {{"order", 3},
                       {"keys", bt.size()},
                       {"nodes", bt.nodeCount()},
                       {"height", bt.height()},
                       {"valid", bt.isValid()}}}
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
