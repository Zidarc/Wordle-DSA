# Part 2: Algorithms

The algorithms live in `backend/include/algorithms/` as templates (so they work on `SafeArray`, plain arrays and strings alike), plus three game classes that are algorithms at heart: `SortingLab`, `Constraints` and `Solver`. Every search and sort takes an **optional counter** so the UI can show how much work it did.

---

## 2.1 Searching an array — `algorithms/Search.h`

All three run on `Dictionary::sortedWords_` (701 words, radix-sorted A→Z).

### Linear search — `linearSearch(a, n, key, &comparisons)`
Check `a[0], a[1], …` until found. **O(n)**, works on unsorted data. ZEBRA (last word) takes 701 comparisons.

### Binary search — `binarySearch(a, low, high, key, &comparisons)` (recursive)
Compare with the middle; recurse into the left or right half. **O(log n)**: 701 words need at most ⌈log2 702⌉ = 10 comparisons. Requires sorted data.

It is **tail recursion**: the recursive call is the last thing done, so it could be a loop. Extra space O(log n) for the call stack (shown as "Extra space" in DSA Insights).

### Interpolation search — `interpolationSearch(a, n, key, &comparisons)`
Instead of the middle, *estimate* where the key should be, like opening a phone book near "Z" for "Zebra":

```
pos = low + (key − a[low]) × (high − low) / (a[high] − a[low])
```

Words are not numbers, so `wordToKey` turns each 5-letter word into a **base-26 number** (`A=0 … Z=25`): `CRANE → ((((2·26+17)·26+0)·26+13)·26+4)`. Sorted words have increasing keys.

- Average **O(log log n)** on evenly spread keys (often 1–4 comparisons here).
- Worst **O(n)** when keys are clumped.

### Timing each lookup — `Dictionary::lookupAll` / `timedLookup`
One lookup takes well under a microsecond, below the clock's resolution. So each method runs once to record its answer and comparison count, then **200 more times** (`Dictionary::kTimingRepeats`) and the total time is divided by 200. A `volatile` variable stops the compiler from deleting the "useless" loop. The result is `LookupResult::nanoseconds`, shown in DSA Insights.

**Why comparisons and time can disagree:** the B-tree makes *more* comparisons than the AVL tree (it scans keys inside a node) but each comparison is on data already loaded together. Hash lookups make 1 comparison but must compute the hash of the whole word first. Big-O describes growth, not exact speed for one small n.

### Choosing the method (`LookupMethod`)
The player picks one of the six in the top bar. `GameSession::makeGuess(…, method)` accepts or rejects the word using `lookupStats.get(method).found`. All six always run (for the comparison table) and always agree; the choice changes the **cost**, never the answer.

| Method | Time (avg) | Worst | Extra space per lookup | Structure space |
|--------|-----------|-------|-----------------------|-----------------|
| Linear | O(n) | O(n) | O(1) | O(n) array |
| Binary | O(log n) | O(log n) | O(log n) recursion | O(n) array |
| Interpolation | O(log log n) | O(n) | O(1) | O(n) array |
| AVL tree | O(log n) | O(log n) | O(log n) recursion | O(n) nodes |
| 2-3 tree | O(log n) | O(log n) | O(1) iterative | O(n) nodes |
| Hash table | O(1) | O(n) | O(1) | O(n + buckets) |

The table is in `Dictionary.cpp` (`kMethods`), sent by `lookupMethodsJson` in `Server.cpp`.

---

## 2.2 Searching a linked list — `algorithms/ListSearch.h`

Lab week 5 asks for searching **linked lists** as well as arrays. Used on the solver's candidate list (`Solver::isCandidate`).

- **Linear:** identical to arrays, O(n).
- **Binary:** a list has no index, so the middle is found with **slow / fast pointers**: `fast` moves 2 nodes for every 1 that `slow` moves; when `fast` hits the end, `slow` is in the middle.

```
A → B → C → D → E → F → null
slow: A B C D        fast: A C E null   → middle = D
```

`ListSearchStats` counts two things:
- **comparisons**: O(log n), same as array binary search;
- **pointer moves**: n/2 + n/4 + … = **O(n)**, because finding each middle means walking.

So binary search on a linked list is no faster than linear overall. It only pays off with **O(1) random access** (arrays). **Interpolation search is impossible** on a list: it needs to jump straight to a computed index.

---

## 2.3 Sorting — `algorithms/Sort.h`

Every sort has the form `xxxSort(arr, n, SortCounter* counter = nullptr)`. `SortCounter` counts **comparisons** and **swaps/moves**, using the helpers `sortLess` and `sortSwap`.

### Elementary sorts (lab week 5)

| Sort | Idea | Comparisons | Swaps | Notes |
|------|------|-------------|-------|-------|
| **Bubble** | swap adjacent pairs out of order; the largest "bubbles" to the end each pass | O(n²) | O(n²) | stops early if a pass swaps nothing → O(n) on sorted input |
| **Selection** | find the minimum of the unsorted part, swap it to the front | always n(n−1)/2 | ≤ n−1 | fewest swaps: good when moving is expensive |
| **Insertion** | take the next element and shift larger ones right until its place is found | O(n²), O(n) if sorted | O(n²) shifts | stable; used by `Wordle` to sort the 5 secret letters before building the BST |
| **Comb** | bubble sort with a gap that starts at n and shrinks ÷1.3 each pass | ~O(n log n) in practice | | moves small values stuck at the end ("turtles") early |

### Advanced sorts (lab week 7)

**Merge sort** — `mergeSort` → `mergeSortRange` (recursive) → `mergeRange`.
Split in half, sort both halves recursively, merge them with a temporary array. Always **O(n log n)**, **stable** (equal elements keep their order), O(n) extra space. **Used by:** the leaderboard (sort the top 10 by score, highest first) — §3.6.

**Quick sort** — `quickSort` → `quickSortRange` → `quickPartition`.
1. **Pivot = median of three** (`a[low]`, `a[mid]`, `a[high]`). Without it, already sorted input (like the dictionary) would give the O(n²) worst case.
2. **Lomuto partition:** move every element < pivot to the left, then put the pivot between the parts. The pivot is now in its final place.
3. Recurse on both sides (**tree recursion**: two calls per level).

Average **O(n log n)**, worst O(n²), not stable, in place.

`quickSortIterative` does the same with an **explicit `ArrayStack<SortRange>`** instead of recursion (simulation of recursion, §2.5). It pushes the **larger** part first so the smaller part is handled next; this keeps the stack O(log n) deep, which `quickSortIterativeKeepsStackShallow` checks.

**Used by:** `Solver::suggestions` ranks possible words by score (§2.6).

**Radix sort (LSD)** — `radixSortWords`.
Every word has exactly 5 letters from 26 symbols, so sort by the **last** letter, then the 4th, …, then the 1st, each pass a stable **counting sort** with 26 buckets. **O(L × (n + 26))**, no comparisons at all. **Used by:** `Dictionary::loadFromStream` to sort the word list once at startup.

---

## 2.4 Sorting Lab — `SortingLab.h/.cpp`

Runs all seven sorts on **the same input** so the numbers are comparable:

1. Constructor: copy the dictionary, shuffle it with Fisher–Yates using a fixed seed (2513), optionally keep only the first `limit` words. Sort a copy with merge sort as the expected answer.
2. `run(name)`: copy the input, run the chosen sort with a `SortCounter`, time it with `steady_clock`, compare with the expected answer (`sortedCorrectly`).
3. `runAll()`: all seven in display order.

Typical result on 701 words: bubble ~245,000 comparisons vs merge ~5,800; radix 0 comparisons. Shown by `GET /api/dsa/sort` → DSA Lab panel.

---

## 2.5 Recursion — `algorithms/Recursion.h` and across the project

| Type | The recursive call… | Example in `Recursion.h` | Also in the project |
|------|--------------------|--------------------------|---------------------|
| **Tail** | is the last thing done | `countLetterTail` | `binarySearch` |
| **Head** | comes first; work happens while returning | `reverseHead` | `BST::destroy` (children first) |
| **Tree** | happens more than once per call | `countOrderings`, `fibonacciTree` | merge sort, quick sort, `btCountNodes`, backtracking |
| **Indirect** | goes through another function | `alternatesFromVowel` ↔ `alternatesFromConsonant` | |
| **Nested** | its argument is itself a recursive call | `mcCarthy91` | |

**Issues with recursion** (lab week 4):
- **No / unreachable base case** → infinite recursion → stack overflow.
- **Repeated work:** `fibonacciTree(10)` makes 177 calls for 11 distinct values.
- **Deep recursion** on big input can overflow the call stack.

**Simulation of recursion** replaces the call stack with your own stack (lab week 9). Three places do it:

| Recursive version | Explicit-stack version | Stack used |
|-------------------|------------------------|------------|
| in-order traversal (conceptually) | `AVLTree::toSortedArray` | `Stack<const Node*>` |
| `quickSort` | `quickSortIterative` | `ArrayStack<SortRange>` |
| `Solver::generateCandidates` | `Solver::generateCandidatesIterative` | `ArrayStack<Frame>` |

The general recipe: each stack entry holds what a call's local variables would have held (the range, or the prefix plus "next letter to try"). Pop an entry = return to that call; push an entry = make a call.

---

## 2.6 Backtracking solver — `Constraints.h/.cpp`, `Solver.h/.cpp`, `FrequencyTable`

### Constraints: what the feedback says
`Constraints::add(guess, feedback)` updates:

| Field | Set by |
|-------|--------|
| `fixed_[pos]` | a green at pos |
| `banned_[pos][letter]` | a yellow or gray at pos |
| `minCount_[letter]` | greens + yellows of that letter in one guess |
| `maxCount_[letter]` | becomes exact when the same guess also has that letter gray |
| `fixed_[pos]` + `minCount_` | a revealed **hint** (`addKnownLetter`): treated as a green that came from no guess |

Example, secret APPLE, guess PUPPY → P: 1 yellow + 1 green + 1 gray ⇒ exactly 2 P's.

Three checks use them: `allowsLetterAt(pos, c)`, `fitsCounts(prefix)` (too many of a letter, or too few places left for letters still needed), `matches(word)`.

### Two ways to find the possible words

1. **Filtering the linked list** (`applyGuess`, and `applyHint` for a revealed hint): `candidates_.removeAllIf(!matches)` — one O(n) pass after each guess or hint.
2. **Backtracking** (`generateCandidates` → `extend`): build words letter by letter.

```
extend(prefix):
    if prefix has 5 letters: record it, return
    for c in 'A'..'Z':
        if feedback rules c out here           → prune
        choose:   prefix += c
        if counts still possible AND some dictionary word starts with prefix:
            explore: extend(prefix)
        else                                   → prune
        un-choose: remove c from prefix        (this is the "backtrack")
```

The prefix check `isDictionaryPrefix` is a **binary search** for the first word ≥ prefix in the sorted array (`prefixChecks` counts its comparisons).

Without pruning there are 26⁵ = 11,881,376 combinations. With pruning, the first call visits about 33,000 letters and after one guess only ~1,000–13,000. Both versions must give **exactly** the candidate list (tested in `backtrackingMatchesCandidateList`).

`generateCandidatesIterative` is the same search with an `ArrayStack<Frame>`, where `Frame = { prefix, next letter to try }`.

### Ranking suggestions
`Solver::suggestions(k)`: build a `FrequencyTable` (5 × 26) from the remaining candidates, score every candidate, **quick sort** the `RankedWord`s (higher score first), keep k. The server only shows suggestions after 2 guesses so they cannot be used to skip the game.

---

## 2.7 KMP, Fisher–Yates and the hash function

### KMP string search — `algorithms/StringSearch.h`
Used for leaderboard name search (`kmpContains(name, query, ignoreCase)`).

Naive search restarts after a mismatch: O(n·m). KMP first builds the **LPS array** (`buildLps`): `lps[i]` = length of the longest proper prefix of `pattern[0..i]` that is also a suffix. On a mismatch after j matches, jump to `j = lps[j−1]` instead of restarting, so the text index never moves backwards: **O(n + m)**.

```
pattern  A B A B C
lps      0 0 1 2 0
```

### Fisher–Yates shuffle — `algorithms/Shuffle.h`
For i from n−1 down to 1: pick j in [0, i] and swap a[i], a[j]. Every permutation is equally likely, O(n). **Used by:** `WordPicker` (secret word order), `GameSession` (hint order), `SortingLab` (input).

### Polynomial rolling hash — `ds/HashTable.h`
See §1.9: `h = h·31 + c` for each character (Horner's rule), then `% bucketCount`.

---

## Self-check questions (Part 2)

1. Why does binary search need at most 10 comparisons for 701 words?
2. How does interpolation search turn a word into a number, and when is it slow?
3. Why is binary search on a linked list no faster overall than linear search?
4. Selection sort on 50 elements: exactly how many comparisons? At most how many swaps?
5. Why does `quickSort` use median-of-three on this project's data in particular?
6. In `quickSortIterative`, why push the larger range first?
7. Radix sort makes 0 comparisons. How does it still sort?
8. Name the recursion type of `binarySearch`, `BST::destroy`, `mergeSortRange` and `mcCarthy91`.
9. In `Solver::extend`, which line is the "backtrack"? Name the three reasons a branch is pruned.
10. Build the LPS array for `AABAA`.
