#ifndef SHUFFLE_H
#define SHUFFLE_H

#include <chrono>
#include <random>

using namespace std;

inline mt19937& shuffleGenerator()
{
    static mt19937 generator(
        (unsigned int)chrono::high_resolution_clock::now().time_since_epoch().count());
    return generator;
}

// GUIDE 2.7: used by WordPicker, hint queue and SortingLab
template <typename Arr>
void fisherYatesWith(Arr& a, int n, mt19937& generator)
{
    for (int i = n - 1; i > 0; i--)
    {
        uniform_int_distribution<int> pick(0, i);
        int j = pick(generator);

        auto temp = a[i];
        a[i] = a[j];
        a[j] = temp;
    }
}


template <typename Arr>
void shuffleArray(Arr& a, int n)
{
    fisherYatesWith(a, n, shuffleGenerator());
}

template <typename Arr>
void shuffleArray(Arr& a, int n, unsigned int seed)
{
    mt19937 generator(seed);
    fisherYatesWith(a, n, generator);
}

#endif