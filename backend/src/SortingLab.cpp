#include "SortingLab.h"

#include "algorithms/Shuffle.h"
#include "algorithms/Sort.h"

#include <chrono>
#include <stdexcept>

namespace
{
const char* const kNames[SortingLab::kAlgorithmCount] = {
    "bubble", "selection", "insertion", "comb", "merge", "quick", "radix"};

const char* const kComplexity[SortingLab::kAlgorithmCount] = {
    "O(n^2)", "O(n^2)", "O(n^2)", "O(n^2 / 2^p)", "O(n log n)", "O(n log n)", "O(L(n + 26))"};

int indexOfAlgorithm(const std::string& name)
{
    for (int i = 0; i < SortingLab::kAlgorithmCount; ++i)
    {
        if (name == kNames[i])
            return i;
    }
    return -1;
}
} // namespace

SortingLab::SortingLab(const SafeArray<std::string>& words, unsigned int seed, std::size_t limit)
{
    SafeArray<std::string> shuffled(words);
    shuffleArray(shuffled, static_cast<int>(shuffled.size()), seed);

    std::size_t n = shuffled.size();
    if (limit != 0 && limit < n)
        n = limit;

    input_.reserve(n);
    for (std::size_t i = 0; i < n; ++i)
        input_.pushBack(shuffled[i]);

    expected_ = input_;
    mergeSort(expected_, static_cast<int>(expected_.size()));
}

const char* SortingLab::algorithmName(int index)
{
    if (index < 0 || index >= kAlgorithmCount)
        throw std::out_of_range("SortingLab::algorithmName: index out of range");
    return kNames[index];
}

// GUIDE 2.4: copy input -> run with counter -> time -> verify
SortRun SortingLab::run(const std::string& algorithm) const
{
    int which = indexOfAlgorithm(algorithm);
    if (which < 0)
        throw std::invalid_argument("SortingLab: unknown algorithm '" + algorithm + "'");

    SortRun result;
    result.algorithm = kNames[which];
    result.complexity = kComplexity[which];

    SafeArray<std::string> data(input_); // every run sorts its own copy
    const int n = static_cast<int>(data.size());
    SortCounter counter;

    auto start = std::chrono::steady_clock::now();
    switch (which)
    {
    case 0:
        bubbleSort(data, n, &counter);
        break;
    case 1:
        selectionSort(data, n, &counter);
        break;
    case 2:
        insertionSort(data, n, &counter);
        break;
    case 3:
        combSort(data, n, &counter);
        break;
    case 4:
        mergeSort(data, n, &counter);
        break;
    case 5:
        quickSort(data, n, &counter);
        break;
    default:
        radixSortWords(data, n, 5, &counter);
        break;
    }
    auto end = std::chrono::steady_clock::now();

    result.comparisons = counter.comparisons;
    result.swaps = counter.swaps;
    result.microseconds =
        std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    result.sortedCorrectly = data.size() == expected_.size();
    for (std::size_t i = 0; result.sortedCorrectly && i < data.size(); ++i)
        result.sortedCorrectly = data[i] == expected_[i];

    return result;
}

SafeArray<SortRun> SortingLab::runAll() const
{
    SafeArray<SortRun> runs;
    for (int i = 0; i < kAlgorithmCount; ++i)
        runs.pushBack(run(kNames[i]));
    return runs;
}
