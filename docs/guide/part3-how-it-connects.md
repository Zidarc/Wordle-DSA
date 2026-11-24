# Part 3: How It All Connects

Parts 1 and 2 explain each structure and algorithm on its own. This part follows the program as it runs: what happens at startup, and what happens in which function every time the player does something.

```
Browser (React)                       C++ backend
───────────────                       ───────────────────────────────────────────
frontend/src/api/client.js  ──HTTP──► Server.cpp route lambda
                                          │
                                          ├─► GameSession (one per game, in a HashTable)
                                          │      ├─ Wordle        (secret word, BST, evaluate)
                                          │      ├─ Board         (2D array)
                                          │      ├─ LetterTracker (array of 26 + jagged rows)
                                          │      ├─ Stack         (history)
                                          │      ├─ Queue         (hint order)
                                          │      └─ DoublyLinkedList (replay)
                                          ├─► Dictionary (array + AVL + 2-3 tree + hash table)
                                          ├─► WordPicker (circular linked list)
                                          ├─► Solver / SortingLab (built per request)
                                          └─► Leaderboard (file + MinHeap + merge sort + KMP)
```

**Rule of the design:** the frontend never decides anything. It sends input and draws what the server returns. It does not know the secret word until the game is over.

---

## 3.1 Startup — `main.cpp` → `Server::Server`

```
main()
 └─ Server server("data/words.txt", "data/scores.txt")
     ├─ leaderboard_(scoresPath)                      just stores the path
     ├─ dictionary_.loadFromFile(wordsPath)
     │   └─ loadFromStream(file)
     │       ├─ for each line: normalize() → isWellFormed()
     │       │     table_.insert(word)  ── false if duplicate → skipped
     │       │     sortedWords_.pushBack(word)
     │       ├─ radixSortWords(sortedWords_)          §2.3, O(5(n+26))
     │       └─ for each word: tree_.insert, btree_.insert   §1.6, §1.8
     ├─ picker_ = WordPicker(dictionary_.sortedWords())
     │   └─ startNewCycle(): Fisher–Yates shuffle → CircularLinkedList   §1.3
     ├─ setupRoutes()      registers /api/game/*, /api/leaderboard, /api/dsa/stats
     │   └─ setupDsaRoutes()  /api/game/solver, /api/dsa/sort, /api/dsa/trees,
     │                        /api/dsa/lookup, /api/games/recent
 └─ server.listen("0.0.0.0", 8080)    blocks; cpp-httplib calls the lambdas
```

**Why the hash table is filled first:** it doubles as the duplicate filter. `insert` returns `false` for a word already present, so each word enters the array once.

**Why radix sort:** all keys are 5 letters A–Z, the perfect case for LSD radix sort, and the sorted array is needed by binary search, interpolation search and the solver's prefix check.

---

## 3.2 Starting a game — `POST /api/game/new`

```
client.newGame(difficulty)
 └─ Server route "/api/game/new"
     ├─ difficultyFromString("hard")              Difficulty.h
     ├─ gameId = generateGameId()                 "game-<counter>-<time>"
     ├─ secret = picker_->nextWord()
     │   ├─ rotation_.current(), rotation_.advance()      circular list
     │   └─ after a full lap: startNewCycle() (reshuffle; never repeat the last word)
     ├─ GameSession session(gameId, secret, difficulty)
     │   ├─ maxAttempts_ = maxAttempts(d)          8 / 6 / 5
     │   ├─ wordle_(secret)                        sorts letters (insertion sort),
     │   │                                         inserts them middle-first into BST<char>
     │   ├─ board_(maxAttempts, 5)                 allocates the 2D arrays
     │   └─ initHintQueue(seed)                    positions 0..4, Fisher–Yates, enqueue
     └─ sessions_.insert(gameId, session)          HashTable<string, GameSession>
```

**Where the Rule of Three matters:** `sessions_.insert` **copies** the `GameSession`. That copy deep-copies the `Board`'s 2D arrays, the `LetterTracker`'s jagged rows, the BST, the stack, the queue and the replay list. If any of those classes had a shallow copy, the local `session` would free memory that the stored copy still uses when the route returns.

---

## 3.3 The life of a guess — `POST /api/game/guess`

This is the most important flow. Follow it in the code from top to bottom.

```
useGame.submit()                                        frontend/src/hooks/useGame.js
 └─ client.sendGuess(gameId, "crane", method)
     └─ Server route "/api/game/guess"                  Server.cpp
         ├─ Dictionary::methodFromString(method)        "avl" → LookupMethod::Avl
         ├─ session = sessions_.find(gameId)            hash table, O(1) avg
         └─ session->makeGuess(guess, dictionary_, method)        GameSession.cpp
             ├─ status must be Playing
             ├─ guess = Dictionary::normalize(raw)       trim + upper-case
             ├─ Dictionary::isWellFormed(guess)          5 letters A–Z?
             ├─ res.lookupStats = dictionary.lookupAll(guess)    all 6 methods, timed   §2.1
             ├─ if !lookupStats.get(method).found → "Not in word list" (no attempt used)
             ├─ Hard mode: every revealed hint letter in its place?
             ├─ fb = wordle_.evaluate(guess)             TWO-PASS COLOURING (below)
             ├─ board_.addRow(fb.guess, fb.states)       2D array row
             ├─ tracker_.update(fb.guess, fb.states)     array of 26, only upgrades
             ├─ history_.push(fb.guess)                  Stack
             ├─ replay_.pushBack(fb)                     DoublyLinkedList
             └─ win?  status = Won,  score = Score::calculate(...)
                full? status = Lost, score = 0
         ├─ game over → recentGames_.enqueue(...)        bounded ArrayQueue
         ├─ Solver solver(dictionary_); replayInto(solver, *session)
         │     → reply.possibleWords = solver.remaining()            §2.6
         └─ JSON: feedback, keyboard rows, letterStatus, lookupStats,
                  lookupMethods (one row per method), methodUsed, status, score,
                  secretWord (only when the game is over)
```

### Two-pass colouring — `Wordle::evaluate`

Repeated letters make Wordle colouring tricky. `remaining[26]` counts each letter of the secret.

1. **Pass 1 (greens):** every letter in the right position → `Correct`, and `remaining[letter]--`.
2. **Pass 2 (yellows / grays):** for every other position, if the BST says the letter is in the word (`containsLetter`) **and** `remaining[letter] > 0` → `Present`, `remaining[letter]--`; otherwise `Absent`.

Secret `APPLE`, guess `PUPPY`:

| Pos | Letter | Pass 1 | Pass 2 | remaining[P] after |
|-----|--------|--------|--------|--------------------|
| 2 | P | green | | 2 → 1 |
| 0 | P | | yellow (1 left) | 1 → 0 |
| 1 | U | | gray (not in word) | |
| 3 | P | | gray (none left) | |
| 4 | Y | | gray | |

Result: 🟨⬜🟩⬜⬜. Doing greens first matters: otherwise the first P would use up a count that the green P needs.

### Score — `Score::calculate`
```
score = ((maxAttempts − attemptsUsed + 1) × 100 − hintsUsed × 50) × multiplier
```
Multiplier 1.0 / 1.5 / 2.0 by difficulty; 0 for a loss; never negative.

---

## 3.4 Hints, history and replay

### `POST /api/game/hint` → `GameSession::useHint`
```
while hintQueue_ not empty:
    pos = hintQueue_.dequeue()
    skip if already revealed, or already green in some board row (board_.stateAt)
    otherwise: reveal secret[pos], hintsUsed_++, return it
```
The queue was shuffled at game start, so hints come in a random but fixed order and never repeat. In Hard mode `makeGuess` then requires that letter in that position.

### `GET /api/game/history` → `GameSession::history()`
Walks the `Stack` from top to bottom with its iterator (nothing is popped): most recent guess first.

### `GET /api/game/replay` → `GameSession::replay()`
Only after game over. Walks the `DoublyLinkedList<GuessFeedback>` from head to tail and returns every step. The game-over dialog steps backwards and forwards through it.

---

## 3.5 Difficulty and Hard mode — `Difficulty.h`

| Level | `maxAttempts` | `scoreMultiplier` | `requiresHintReuse` |
|-------|---------------|-------------------|---------------------|
| Easy | 8 | 1.0 | no |
| Normal | 6 | 1.5 | no |
| Hard | 5 | 2.0 | yes |

The number of attempts sets the number of **rows of the 2D board** (`board_(maxAttempts, 5)`).

---

## 3.6 Leaderboard — `Leaderboard.cpp`

### Saving: `POST /api/leaderboard` → `Leaderboard::addScore`
Only for won games. Appends `name|score|difficulty|date` to `data/scores.txt` (`|` and newlines in names are replaced with spaces).

### Reading: `GET /api/leaderboard?search=` → `Leaderboard::topScores(filter)`
```
loadAll()                         read every line of scores.txt into a SafeArray
filter with kmpContains(name, query, ignoreCase)      KMP, §2.7
MinHeap<Entry, MinScoreCompare> heap
for each entry: heap.pushBounded(entry, 10)            top-K, §1.10
pop all 10 into a SafeArray
mergeSort(top)                    LeaderboardEntry::operator< = higher score first
```
**Why the heap:** m saved scores cost O(m log 10) instead of O(m log m) for sorting everything. Only 10 entries are then merge sorted.

---

## 3.7 The DSA routes

| Route | Builds | Returns |
|-------|--------|---------|
| `GET /api/dsa/stats` | — | `Dictionary::stats()`: word count, AVL height and leaves, 2-3 tree height and nodes, hash buckets, load factor, longest chain, rehashes |
| `GET /api/dsa/lookup?word=&method=` | — | `lookupAll` rows: comparisons, ns, Big-O, extra space, which method decides |
| `GET /api/dsa/trees` | — | `Dictionary::avlShape()` (full / complete / perfect, leaves, heights) and the 2-3 tree's shape |
| `GET /api/dsa/sort?limit=` | `SortingLab` | 7 runs: comparisons, moves, µs, `sortedCorrectly` |
| `GET /api/game/solver?gameId=` | `Solver` + `replayInto` | possible words, backtracking stats (recursive vs explicit stack), suggestions after 2 guesses |
| `GET /api/games/recent` | — | `recentGames_` (bounded `ArrayQueue`), newest first |

`Solver` holds a **reference** to the dictionary, so it cannot be stored inside a `GameSession` (sessions are copied into the hash table). Instead the server rebuilds it from the session's replay list whenever needed (`replayInto`): cheap, because it is one pass over ≤ 8 guesses.

---

## 3.8 Server details — `Server.h/.cpp`

- **cpp-httplib** and **nlohmann/json** are used **only** in `Server.cpp`. CMake compiles `Server.cpp` into the `wordle_server` executable only, so the tests (which link `wordle_core`) never need them.
- Each route is a lambda: parse JSON → find the session → call one game method → build JSON.
- `setCors` adds CORS headers; in development the Vite proxy (`frontend/vite.config.js`) forwards `/api` to port 8080 anyway.
- Errors: 400 for bad input (with the reason in `"error"`), 404 for an unknown `gameId`.

---

## 3.9 Call map: who calls whom

Read "A → B" as "A calls B".

```
Server routes
├─ /game/new ─────► WordPicker::nextWord ─► CircularLinkedList::current/advance
│                   GameSession::GameSession ─► Wordle::Wordle ─► insertionSort, BST::insert
│                                              Board::Board, initHintQueue ─► shuffleArray, Queue::enqueue
│                   HashTable::insert ─► KeyHash, SinglyLinkedList::pushFront, rehash
├─ /game/guess ───► HashTable::find
│                   GameSession::makeGuess ─► Dictionary::normalize / isWellFormed / lookupAll
│                   │                          lookupAll ─► linearSearch, binarySearch,
│                   │                                       interpolationSearch(wordToKey),
│                   │                                       AVLTree::contains, BTree::search,
│                   │                                       HashTable::contains   (each via timedLookup)
│                   │                          Wordle::evaluate ─► BST::contains
│                   │                          Board::addRow, LetterTracker::update,
│                   │                          Stack::push, DoublyLinkedList::pushBack, Score::calculate
│                   ArrayQueue::enqueue (recent games)
│                   Solver::applyGuess ─► Constraints::add, SinglyLinkedList::removeAllIf ─► Constraints::matches
├─ /game/hint ────► GameSession::useHint ─► Queue::dequeue, Board::stateAt
├─ /game/history ─► Stack iterator
├─ /game/replay ──► DoublyLinkedList iterator
├─ /game/solver ──► Solver::applyGuess, generateCandidates ─► extend ─► isDictionaryPrefix
│                   generateCandidatesIterative ─► ArrayStack, isDictionaryPrefix
│                   suggestions ─► FrequencyTable::build/score, quickSort ─► quickPartition
├─ /leaderboard ──► Leaderboard::topScores ─► loadAll, kmpContains ─► buildLps,
│                                            MinHeap::pushBounded / pop, mergeSort
├─ POST /leaderboard ► Leaderboard::addScore
├─ /dsa/stats ────► Dictionary::stats ─► AVLTree::height/leafCount/isBalanced, BTree::height,
│                                       HashTable::loadFactor/longestChain/usedBuckets
├─ /dsa/lookup ───► Dictionary::lookupAll, methodInfo
├─ /dsa/trees ────► AVLTree::shape ─► btShape ─► btIsFull, btIsComplete(Queue), btIsPerfect
│                   BTree::isValid ─► validate
├─ /dsa/sort ─────► SortingLab::SortingLab ─► shuffleArray, mergeSort
│                   SortingLab::runAll ─► run ─► bubble/selection/insertion/comb/merge/quick/radix
└─ /games/recent ─► ArrayQueue::at
```

---

## 3.10 Where to find X

| I want to see… | Go to |
|----------------|-------|
| Why a letter turned yellow / gray | `Wordle::evaluate` |
| Where "Not in word list" is decided | `GameSession::makeGuess` (`lookupStats.get(method)`) |
| How each lookup is timed | `timedLookup` in `Dictionary.cpp` |
| The Big-O table for the six lookups | `kMethods` in `Dictionary.cpp` |
| How the secret word is chosen | `WordPicker::nextWord`, `startNewCycle` |
| Where hard mode is enforced | `GameSession::makeGuess` (Hard mode block) |
| How the score is computed | `Score::calculate` |
| Where games are stored | `Server::sessions_` (`HashTable<string, GameSession>`) |
| AVL rotations | `AVLTree::balance`, `rotateLeft`, `rotateRight` |
| B-tree split | `BTree::splitChild` |
| Rehashing | `HashTable::rehash` |
| Backtracking | `Solver::extend` (recursive), `generateCandidatesIterative` |
| Quick sort partition | `quickPartition` in `Sort.h` |
| Top-10 leaderboard | `Leaderboard::topScores` |
| Every JSON field the UI gets | the route lambdas in `Server.cpp`, documented in `docs/API.md` |

---

## 3.11 Study plan (one night)

| Time | Do |
|------|----|
| 30 min | §3.1 – §3.3 with `Server.cpp`, `GameSession.cpp`, `Wordle.cpp` open side by side |
| 60 min | Part 1 §1.1 – §1.4, run `test_safe_array`, `test_linked_lists`, `test_stack_queue`, `test_array_stack_queue` |
| 60 min | Part 1 §1.5 – §1.10, run the tree, B-tree, hash and heap tests |
| 60 min | Part 2 §2.1 – §2.5, play a game and compare the DSA Insights numbers with the theory |
| 45 min | Part 2 §2.6 and Part 3 §3.4 – §3.9, open the Solver and DSA Lab tabs |
| 15 min | Answer the self-check questions of all three parts |

## Self-check questions (Part 3)

1. List, in order, every data structure a single accepted guess touches.
2. Why does an invalid word not use up an attempt? Which line decides it?
3. Why must `GameSession`'s members implement the Rule of Three correctly for the server to work?
4. Where does the secret word come from, and why can it not repeat until every word is used?
5. Which three algorithms does `GET /api/leaderboard?search=ali` run?
6. Why is the `Solver` rebuilt for every request instead of stored in the session?
7. A player picks "linear" as the lookup method. What changes in the result, and what does not?
8. Trace `PUPPY` against `APPLE` through both passes of `evaluate` without looking.
