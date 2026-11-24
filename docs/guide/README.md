# Wordle-DSA Study Guide

This guide explains **what** every part of the backend does, **why** it was built that way, **where** the code is, and **how the functions call each other**. The frontend is only mentioned where it calls the backend: the DSA is the point.

## The three parts

| Part | File | Covers | Read it to answer |
|------|------|--------|-------------------|
| 1 | [Part 1: Data Structures](part1-data-structures.md) | Everything in `backend/include/ds/`, plus the 2D / jagged arrays in `Board`, `LetterTracker` and `FrequencyTable` | "How is X stored, what does each operation cost, and why this structure?" |
| 2 | [Part 2: Algorithms](part2-algorithms.md) | Everything in `backend/include/algorithms/`, plus `SortingLab`, `Constraints` and `Solver` | "How does this algorithm work step by step, and how many comparisons does it make?" |
| 3 | [Part 3: How It All Connects](part3-how-it-connects.md) | `Dictionary`, `WordPicker`, `Wordle`, `GameSession`, `Score`, `Leaderboard`, `Server`: the life of every request | "What happens, in which function, when the player clicks something?" |

The parts can be studied by three people in parallel. Each part ends with **self-check questions**; answer them without looking, then check the code.

## How the code points back here

Comments in the code are short tips, not explanations. They look like this:

```cpp
// GUIDE 1.6: AVL rotations (LL, RR, LR, RL)
```

`GUIDE 1.6` means section **1.6** of Part 1. Search the repository for `GUIDE` to see every tip:

```bash
git grep -n "GUIDE"
```

## Recommended reading order

1. **Part 3, §3.1 – §3.3** first (startup, new game, life of a guess). This gives you the big picture and tells you which structures matter most.
2. **Part 1** in order: each structure builds on the previous one (SafeArray → linked lists → stack/queue → trees → hash table → heap).
3. **Part 2** in order: searches → sorts → recursion → backtracking.
4. **Part 3** again from §3.4, now that every structure is familiar.
5. Run the tests while reading: every structure and algorithm has its own `tests/test_*.cpp`, and the tests are the quickest way to see the expected behaviour.

```bash
cmake -B build -G Ninja
cmake --build build
ctest --test-dir build --output-on-failure
```

## One-page map: file → guide section

| File | Section |
|------|---------|
| `ds/SafeArray.h` | 1.2 |
| `ds/SinglyLinkedList.h`, `DoublyLinkedList.h`, `CircularLinkedList.h` | 1.3 |
| `ds/Stack.h`, `Queue.h`, `ArrayStack.h`, `ArrayQueue.h` | 1.4 |
| `ds/BST.h` | 1.5 |
| `ds/AVLTree.h` | 1.6 |
| `ds/BinaryTreeProperties.h` | 1.7 |
| `ds/BTree.h` | 1.8 |
| `ds/HashTable.h` | 1.9 |
| `ds/MinHeap.h` | 1.10 |
| `Board`, `LetterTracker`, `FrequencyTable` | 1.11 |
| `algorithms/Search.h` | 2.1 |
| `algorithms/ListSearch.h` | 2.2 |
| `algorithms/Sort.h`, `SortingLab` | 2.3, 2.4 |
| `algorithms/Recursion.h` | 2.5 |
| `Constraints`, `Solver` | 2.6 |
| `algorithms/StringSearch.h` (KMP), `Shuffle.h` | 2.7 |
| `main.cpp`, `Server` | 3.1, 3.8 |
| `Dictionary` | 3.1, 3.3 |
| `WordPicker` | 3.2 |
| `Wordle` (two-pass evaluate) | 3.3 |
| `GameSession`, `Difficulty`, `Score` | 3.2 – 3.5 |
| `Leaderboard` | 3.6 |
| Call map: who calls whom | 3.9 |

## Glossary

| Term | Meaning here |
|------|--------------|
| `n` | Number of words in the dictionary (701 in `data/words.txt`) |
| `L` | Word length (always 5) |
| Comparison | One `==` or `<` between two elements; every search and sort counts them |
| Rule of Three | A class that owns memory defines destructor, copy constructor and copy assignment |
| Deep copy | Copying the data the pointers point to, not just the pointers |
| Load factor | Hash table entries ÷ buckets |
| Pruning | Abandoning a branch of a search as soon as it cannot lead to an answer |
