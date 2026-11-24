#include "Leaderboard.h"

#include "algorithms/Sort.h"
#include "algorithms/StringSearch.h"

#include <fstream>
#include <sstream>

struct MinScoreCompare
{
    bool operator()(const LeaderboardEntry& a, const LeaderboardEntry& b) const
    {
        return a.score < b.score;
    }
};

Leaderboard::Leaderboard(const std::string& filePath)
    : filePath_(filePath)
{
}

std::string Leaderboard::cleanName(const std::string& name)
{
    std::string safe = name;
    for (char& c : safe)
    {
        if (c == '|' || c == '\n' || c == '\r' || c == '\t')
            c = ' ';
    }
    std::size_t first = safe.find_first_not_of(' ');
    if (first == std::string::npos)
        return "Anonymous";
    std::size_t last = safe.find_last_not_of(' ');
    safe = safe.substr(first, last - first + 1);
    if (safe.size() > kMaxNameLength)
    {
        safe.resize(kMaxNameLength);
        safe.erase(safe.find_last_not_of(' ') + 1);
    }
    return safe;
}

bool Leaderboard::addScore(const std::string& name, int score,
                           const std::string& difficulty, const std::string& date)
{
    std::ofstream file(filePath_, std::ios::app);
    if (!file)
        return false;

    std::string safeName = cleanName(name);

    file << safeName << "|" << score << "|" << difficulty << "|" << date << "\n";
    return true;
}

SafeArray<LeaderboardEntry> Leaderboard::loadAll() const
{
    SafeArray<LeaderboardEntry> entries;
    std::ifstream file(filePath_);
    if (!file)
        return entries;

    std::string line;
    while (std::getline(file, line))
    {
        if (line.empty())
            continue;

        std::stringstream ss(line);
        std::string name, scoreStr, diff, date;

        if (std::getline(ss, name, '|') &&
            std::getline(ss, scoreStr, '|') &&
            std::getline(ss, diff, '|') &&
            std::getline(ss, date))
        {
            try
            {
                LeaderboardEntry entry;
                entry.name = name;
                entry.score = std::stoi(scoreStr);
                entry.difficulty = diff;
                entry.date = date;
                entries.pushBack(entry);
            }
            catch (...)
            {
                // Skip corrupted line
            }
        }
    }
    return entries;
}

// GUIDE 3.6: KMP filter -> MinHeap top 10 -> merge sort
SafeArray<LeaderboardEntry> Leaderboard::topScores(const std::string& nameFilter) const
{
    SafeArray<LeaderboardEntry> all = loadAll();
    SafeArray<LeaderboardEntry> filtered;

    for (std::size_t i = 0; i < all.size(); ++i)
    {
        if (nameFilter.empty() || kmpContains(all[i].name, nameFilter, true))
            filtered.pushBack(all[i]);
    }

    if (filtered.empty())
        return filtered;

    // Stream through MinHeap to get top 10
    MinHeap<LeaderboardEntry, MinScoreCompare> heap;
    for (std::size_t i = 0; i < filtered.size(); ++i)
        heap.pushBounded(filtered[i], kTopCount);

    SafeArray<LeaderboardEntry> top;
    while (!heap.empty())
    {
        top.pushBack(heap.top());
        heap.pop();
    }

    // Sort descending using merge sort
    mergeSort(top, static_cast<int>(top.size()));
    return top;
}
