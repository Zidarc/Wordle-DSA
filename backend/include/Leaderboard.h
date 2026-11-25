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

using namespace std;

struct LeaderboardEntry
{
    string name;
    int score = 0;
    string difficulty;
    string date;

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
    static constexpr size_t kTopCount = 10;

    explicit Leaderboard(const string& filePath = "data/scores.txt");

    // Appends a new score record to the file.
    // The name as it is stored: '|' and line breaks become spaces (they
    // would break the file format), spaces are trimmed from both ends, at
    // most kMaxNameLength characters are kept, and an empty name becomes
    // "Anonymous".
    static constexpr size_t kMaxNameLength = 20;
    static string cleanName(const string& name);

    bool addScore(const string& name, int score,
                  const string& difficulty, const string& date);

    // Returns top 10 scores (heap + merge sort), optionally filtered by player name (KMP).
    SafeArray<LeaderboardEntry> topScores(const string& nameFilter = "") const;

    // All raw scores from the file.
    SafeArray<LeaderboardEntry> loadAll() const;

    const string& filePath() const { return filePath_; }

private:
    string filePath_;
};

#endif // WORDLE_DSA_LEADERBOARD_H
