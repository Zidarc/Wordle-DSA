#ifndef WORDLE_DSA_LEADERBOARD_H
#define WORDLE_DSA_LEADERBOARD_H

// Leaderboard: manages saved game scores.
//
// Top-K selection: all scores from data/scores.txt are streamed through a
// MinHeap of size K = 10, then sorted in descending order using mergeSort.
// Name search: filters player names using KMP string search.

#include "ds/MinHeap.h"
#include "ds/SafeArray.h"

#include <cstddef>
#include <string>

struct LeaderboardEntry
{
    std::string name;
    int score = 0;
    std::string difficulty;
    std::string date;

    // For mergeSort: higher score comes first (descending).
    bool operator<(const LeaderboardEntry& other) const
    {
        if (score != other.score)
            return score > other.score;
        return name < other.name;
    }

    bool operator==(const LeaderboardEntry& other) const
    {
        return name == other.name && score == other.score &&
               difficulty == other.difficulty && date == other.date;
    }
};

class Leaderboard
{
public:
    static constexpr std::size_t kTopCount = 10;

    explicit Leaderboard(const std::string& filePath = "data/scores.txt");

    // Appends a new score record to the file.
    bool addScore(const std::string& name, int score,
                  const std::string& difficulty, const std::string& date);

    // Returns top 10 scores (heap + merge sort), optionally filtered by player name (KMP).
    SafeArray<LeaderboardEntry> topScores(const std::string& nameFilter = "") const;

    // All raw scores from the file.
    SafeArray<LeaderboardEntry> loadAll() const;

    const std::string& filePath() const { return filePath_; }

private:
    std::string filePath_;
};

#endif // WORDLE_DSA_LEADERBOARD_H
