// Tests for FrequencyTable, Constraints and Solver (backtracking, candidate
// list, ranking).

#include "TestFramework.h"

#include "Constraints.h"
#include "FrequencyTable.h"
#include "Solver.h"
#include "Wordle.h"

#include <stdexcept>
#include <string>

static const std::string kWordsFile = std::string(WORDLE_DATA_DIR) + "/words.txt";

static Dictionary realDictionary()
{
    Dictionary d;
    d.loadFromFile(kWordsFile);
    return d;
}

static bool sameWords(const SafeArray<std::string>& a, const SinglyLinkedList<std::string>& b)
{
    if (a.size() != b.size())
        return false;
    std::size_t i = 0;
    for (const std::string& w : b)
    {
        if (a[i++] != w)
            return false;
    }
    return true;
}

// ----- FrequencyTable -----

TEST_CASE(frequencyTableCountsPositionsAndWords)
{
    SafeArray<std::string> words;
    words.pushBack("APPLE");
    words.pushBack("PAPER");
    FrequencyTable t;
    t.build(words);
    CHECK_EQ(t.wordCount(), 2);
    CHECK_EQ(t.countAt(0, 'A'), 1);
    CHECK_EQ(t.countAt(2, 'P'), 2);
    CHECK_EQ(t.wordsContaining('P'), 2); // counted once per word
    CHECK_EQ(t.wordsContaining('Z'), 0);
    CHECK_THROWS(t.countAt(5, 'A'), std::out_of_range);
    CHECK_THROWS(t.wordsContaining('1'), std::out_of_range);
}

TEST_CASE(frequencyTableScoresRepeatedLettersOnce)
{
    SafeArray<std::string> words;
    words.pushBack("CRANE");
    words.pushBack("CRATE");
    FrequencyTable t;
    t.build(words);
    CHECK(t.score("CRANE") > t.score("CCCCC"));
    CHECK_EQ(t.score("ABC"), 0LL);
}

TEST_CASE(frequencyTableCopyIsDeep)
{
    FrequencyTable a;
    a.add("CRANE");
    FrequencyTable b(a);
    b.add("CRANE");
    CHECK_EQ(a.countAt(0, 'C'), 1);
    CHECK_EQ(b.countAt(0, 'C'), 2);
    FrequencyTable c;
    c = b;
    CHECK_EQ(c.wordCount(), 2);
}

// ----- Constraints -----

TEST_CASE(constraintsFromRepeatedLetters)
{
    Wordle w("APPLE");
    GuessFeedback fb = w.evaluate("PUPPY"); // P yellow, U gray, P green, P gray, Y gray
    Constraints c;
    c.add("PUPPY", fb.states);
    CHECK_EQ(c.fixedAt(2), 'P');
    CHECK_EQ(c.minCount('P'), 2);
    CHECK_EQ(c.maxCount('P'), 2); // a gray P makes the count exact
    CHECK_EQ(c.maxCount('U'), 0);
    CHECK(!c.allowsLetterAt(0, 'P')); // yellow: not here
    CHECK(c.matches("APPLE"));
    CHECK(!c.matches("PPPLE"));
}

TEST_CASE(constraintsPruneShortPrefixes)
{
    Wordle w("CRANE");
    GuessFeedback fb = w.evaluate("SLATE"); // A green, E green
    Constraints c;
    c.add("SLATE", fb.states);
    CHECK(!c.allowsLetterAt(0, 'S'));
    CHECK(c.allowsLetterAt(2, 'A'));
    CHECK(!c.allowsLetterAt(2, 'O'));
    CHECK(c.fitsCounts("CR"));
    CHECK(!c.fitsCounts("CSS")); // S is gray: none allowed
}

TEST_CASE(constraintsRejectBadInput)
{
    Constraints c;
    LetterState s[5] = {LetterState::Absent, LetterState::Absent, LetterState::Absent,
                        LetterState::Absent, LetterState::Absent};
    CHECK_THROWS(c.add("abc", s), std::invalid_argument);
    CHECK_THROWS(c.add("crane", s), std::invalid_argument);
    CHECK_THROWS(c.add("CRANE", nullptr), std::invalid_argument);
}

// ----- Solver -----

TEST_CASE(solverStartsWithWholeDictionary)
{
    Dictionary d = realDictionary();
    Solver s(d);
    CHECK_EQ(s.remaining(), d.size());
    CHECK(s.isCandidate("crane"));
    CHECK(!s.isCandidate("QQQQQ"));
}

TEST_CASE(solverNarrowsDownAndKeepsTheSecret)
{
    Dictionary d = realDictionary();
    Solver s(d);
    Wordle w("APPLE");
    const char* guesses[] = {"CRANE", "SLATE", "PUPPY"};
    std::size_t last = s.remaining();
    for (const char* g : guesses)
    {
        GuessFeedback fb = w.evaluate(g);
        int removed = s.applyGuess(g, fb.states);
        CHECK(removed >= 0);
        CHECK(s.remaining() <= last);
        CHECK(s.isCandidate("APPLE"));
        last = s.remaining();
    }
    CHECK(s.remaining() < 10u);
}

TEST_CASE(backtrackingMatchesCandidateList)
{
    Dictionary d = realDictionary();
    Solver s(d);
    CHECK(sameWords(s.generateCandidates(), s.candidates()));

    Wordle w("GHOST");
    GuessFeedback fb = w.evaluate("CRANE");
    s.applyGuess("CRANE", fb.states);
    BacktrackStats rec;
    BacktrackStats iter;
    SafeArray<std::string> a = s.generateCandidates(&rec);
    SafeArray<std::string> b = s.generateCandidatesIterative(&iter);
    CHECK(sameWords(a, s.candidates()));
    CHECK(sameWords(b, s.candidates()));
    CHECK_EQ(rec.nodesVisited, iter.nodesVisited);
    CHECK_EQ(rec.pruned, iter.pruned);
    CHECK_EQ(rec.solutions, static_cast<int>(s.remaining()));
}

TEST_CASE(backtrackingPrunesMostOfTheSearchSpace)
{
    Dictionary d = realDictionary();
    Solver s(d);
    BacktrackStats st;
    s.generateCandidates(&st);
    CHECK(st.nodesVisited < 26LL * 26 * 26 * 26 * 26 / 100); // far below 26^5
    CHECK(st.pruned > 0);
    CHECK_EQ(st.maxDepth, 5);
}

TEST_CASE(prefixCheckUsesBinarySearch)
{
    Dictionary d = realDictionary();
    Solver s(d);
    long long comparisons = 0;
    CHECK(s.isDictionaryPrefix("CR", &comparisons));
    CHECK(comparisons <= 11);
    CHECK(!s.isDictionaryPrefix("QX"));
    CHECK(s.isDictionaryPrefix(""));
}

TEST_CASE(suggestionsAreRankedAndPossible)
{
    Dictionary d = realDictionary();
    Solver s(d);
    SafeArray<RankedWord> top = s.suggestions(5);
    CHECK_EQ(top.size(), 5u);
    for (std::size_t i = 1; i < top.size(); ++i)
        CHECK(top[i - 1].score >= top[i].score);
    for (std::size_t i = 0; i < top.size(); ++i)
        CHECK(s.isCandidate(top[i].word));
}

TEST_CASE(resetRestoresEverything)
{
    Dictionary d = realDictionary();
    Solver s(d);
    Wordle w("APPLE");
    s.applyGuess("CRANE", w.evaluate("CRANE").states);
    s.reset();
    CHECK_EQ(s.remaining(), d.size());
    CHECK_EQ(s.constraints().guessesAdded(), 0);
}

int main()
{
    return tf::runAll("Solver");
}
