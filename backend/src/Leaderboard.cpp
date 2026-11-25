#include "Leaderboard.h"

#include "algorithms/Sort.h"
#include "algorithms/StringSearch.h"

#include <fstream>
#include <sstream>

using namespace std;

struct MinScoreCompare
{
    bool operator()(const LeaderboardEntry& a, const LeaderboardEntry& b) const
    {
        return a.score < b.score;
    }
};

Leaderboard::Leaderboard(const string& filePath)
    : filePath_(filePath)
{
}

string Leaderboard::cleanName(const string& name)
{
    string safe = name;
    for (char& c : safe)
    {
        if (c == '|' || c == '\n' || c == '\r' || c == '\t')
            c = ' ';
    }
    size_t first = safe.find_first_not_of(' ');
    if (first == string::npos)
        return "Anonymous";
    size_t last = safe.find_last_not_of(' ');
    safe = safe.substr(first, last - first + 1);
    if (safe.size() > kMaxNameLength)
    {
        safe.resize(kMaxNameLength);
        safe.erase(safe.find_last_not_of(' ') + 1);
    }
    return safe;
}

bool Leaderboard::addScore(const string& name, int score,
                           const string& difficulty, const string& date)
{
    ofstream file(filePath_, ios::app);
    if (!file)
        return false;

    string safeName = cleanName(name);

    file << safeName << "|" << score << "|" << difficulty << "|" << date << "\n";
    return true;
}

SafeArray<LeaderboardEntry> Leaderboard::loadAll() const
{
    SafeArray<LeaderboardEntry> entries;
    ifstream file(filePath_);
    if (!file)
        return entries;

    string line;
    while (getline(file, line))
    {
        if (line.empty())
            continue;

        stringstream ss(line);
        string name, scoreStr, diff, date;

        if (getline(ss, name, '|') &&
            getline(ss, scoreStr, '|') &&
            getline(ss, diff, '|') &&
            getline(ss, date))
        {
            try
            {
                LeaderboardEntry entry;
                entry.name = name;
                entry.score = stoi(scoreStr);
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
SafeArray<LeaderboardEntry> Leaderboard::topScores(const string& nameFilter) const
{
    SafeArray<LeaderboardEntry> all = loadAll();
    SafeArray<LeaderboardEntry> filtered;

    for (size_t i = 0; i < all.size(); ++i)
    {
        if (nameFilter.empty() || kmpContains(all[i].name, nameFilter, true))
            filtered.pushBack(all[i]);
    }

    if (filtered.empty())
        return filtered;

    // Stream through MinHeap to get top 10
    MinHeap<LeaderboardEntry, MinScoreCompare> heap;
    for (size_t i = 0; i < filtered.size(); ++i)
        heap.pushBounded(filtered[i], kTopCount);

    SafeArray<LeaderboardEntry> top;
    while (!heap.empty())
    {
        top.pushBack(heap.top());
        heap.pop();
    }

    // Sort descending using merge sort
    mergeSort(top, (int)(top.size()));
    return top;
}
