// Tests for SortingLab: every algorithm sorts the same input correctly and
// the counters behave the way the theory says they should.

#include "TestFramework.h"

#include "Dictionary.h"
#include "SortingLab.h"

#include <sstream>
#include <stdexcept>
#include <string>

static SafeArray<std::string> sampleWords()
{
    std::istringstream in("CRANE\nSLATE\nAPPLE\nBRAVE\nAUDIO\nPLUMB\nGHOST\nTRAIN\nZEBRA\nMOUSE\n"
                          "HOUSE\nLEMON\nRIVER\nSTONE\nCLOUD\nFLAME\nGRAPE\nPIANO\nQUEEN\nWATER\n");
    Dictionary d;
    d.loadFromStream(in);
    return d.sortedWords();
}

TEST_CASE(inputIsAShuffledCopy)
{
    SafeArray<std::string> words = sampleWords();
    SortingLab lab(words, 7);
    CHECK_EQ(lab.inputSize(), words.size());

    bool differs = false;
    for (std::size_t i = 0; i < words.size(); ++i)
        differs = differs || lab.input()[i] != words[i];
    CHECK(differs); // a shuffle of 20 words is almost never the identity
}

TEST_CASE(sameSeedGivesSameInput)
{
    SafeArray<std::string> words = sampleWords();
    SortingLab a(words, 99);
    SortingLab b(words, 99);
    bool same = true;
    for (std::size_t i = 0; i < a.inputSize(); ++i)
        same = same && a.input()[i] == b.input()[i];
    CHECK(same);
}

TEST_CASE(limitTakesOnlyPartOfTheInput)
{
    SortingLab lab(sampleWords(), 1, 8);
    CHECK_EQ(lab.inputSize(), 8u);
    SafeArray<SortRun> runs = lab.runAll();
    for (std::size_t i = 0; i < runs.size(); ++i)
        CHECK(runs[i].sortedCorrectly);
}

TEST_CASE(allSevenAlgorithmsSortCorrectly)
{
    SortingLab lab(sampleWords());
    SafeArray<SortRun> runs = lab.runAll();
    CHECK_EQ(runs.size(), static_cast<std::size_t>(SortingLab::kAlgorithmCount));
    for (std::size_t i = 0; i < runs.size(); ++i)
    {
        CHECK(runs[i].sortedCorrectly);
        CHECK_EQ(runs[i].algorithm, std::string(SortingLab::algorithmName(static_cast<int>(i))));
        CHECK(!runs[i].complexity.empty());
    }
}

TEST_CASE(countersMatchTheTheory)
{
    SortingLab lab(sampleWords());
    const long long n = static_cast<long long>(lab.inputSize());

    SortRun selection = lab.run("selection");
    CHECK_EQ(selection.comparisons, n * (n - 1) / 2);
    CHECK(selection.swaps <= n - 1);

    SortRun radix = lab.run("radix");
    CHECK_EQ(radix.comparisons, 0LL);
    CHECK_EQ(radix.swaps, 5 * n);

    SortRun bubble = lab.run("bubble");
    SortRun merge = lab.run("merge");
    CHECK(merge.comparisons < bubble.comparisons);
}

TEST_CASE(unknownAlgorithmThrows)
{
    SortingLab lab(sampleWords());
    CHECK_THROWS(lab.run("bogo"));
    CHECK_THROWS(SortingLab::algorithmName(7));
}

TEST_CASE(fullDictionaryRuns)
{
    Dictionary d;
    d.loadFromFile(std::string(WORDLE_DATA_DIR) + "/words.txt");
    SortingLab lab(d.sortedWords());
    SafeArray<SortRun> runs = lab.runAll();
    for (std::size_t i = 0; i < runs.size(); ++i)
        CHECK(runs[i].sortedCorrectly);
}

int main()
{
    return tf::runAll("SortingLab");
}
