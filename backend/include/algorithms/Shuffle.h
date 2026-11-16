#ifndef SHUFFLE_H
#define SHUFFLE_H

#include <chrono>
#include <random>
#include <type_traits>

inline std::mt19937& shuffleGenerator()
{
    static std::mt19937 generator(
        (unsigned int)std::chrono::high_resolution_clock::now().time_since_epoch().count());
    return generator;
}

template <typename Arr>
void fisherYatesWith(Arr& a, int n, std::mt19937& generator)
{
    typedef typename std::decay<decltype(a[0])>::type T;
    for (int i = n - 1; i > 0; i--)
    {
        std::uniform_int_distribution<int> pick(0, i);
        int j = pick(generator);

        T temp = a[i];
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
    std::mt19937 generator(seed);
    fisherYatesWith(a, n, generator);
}

#endif