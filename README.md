# Wordle-DSA

**A Wordle word-guessing game built on hand-implemented Data Structures, with a C++ backend and a React web frontend.**

Data Structures course project, FAST-NUCES Karachi, Section K.

The player has a limited number of attempts to guess a hidden five-letter word. After each guess, every letter is marked as:

| Colour | Meaning |
|--------|---------|
| 🟩 Green | Letter is in the word and in the correct position |
| 🟨 Yellow | Letter is in the word but in a different position |
| ⬜ Gray | Letter is not in the word |

The game logic, word validation, hints, history, scoring and leaderboard are all implemented in C++ using **data structures written by the team**: linked lists, stacks, queues, BST, AVL tree, hash table, heap, dynamic 2D arrays, and several searching and sorting algorithms. A React web app shows the board, the on-screen keyboard, the leaderboard and a **DSA Insights** panel, and talks to the C++ backend over a small local HTTP API.

---

## Table of Contents

1. [Team](#1-team)
2. [Project Purpose](#2-project-purpose)
3. [Reference System](#3-reference-system)
4. [Changes from the Proposal](#4-changes-from-the-proposal)
5. [Features](#5-features)
6. [Data Structures and Algorithms Used](#6-data-structures-and-algorithms-used)
7. [Course Topic Coverage](#7-course-topic-coverage)
8. [System Architecture](#8-system-architecture)
9. [Game Flow](#9-game-flow)
10. [Game Rules](#10-game-rules)
11. [Backend API (Draft)](#11-backend-api-draft)
12. [Project Folder Structure](#12-project-folder-structure)
13. [Development Environment](#13-development-environment)
14. [Building and Running](#14-building-and-running)
15. [Git Branch Strategy](#15-git-branch-strategy)
16. [16-Day Schedule](#16-16-day-schedule)
17. [Testing Requirements](#17-testing-requirements)
18. [Complexity Overview](#18-complexity-overview)
19. [Final Demonstration](#19-final-demonstration)
20. [Out of Scope](#20-out-of-scope)
21. [Learning Outcomes](#21-learning-outcomes)
22. [Credits and License](#22-credits-and-license)

---

## 1. Team

| Roll No. | Name |
|----------|------|
| 25K-2513 | Ali Hussain |
| 25K-3011 | Ali |
| 25K-0906 | M. Abbas |

See [CONTRIBUTING.md](CONTRIBUTING.md) for the workflow, branch rules and daily handoff format.

---

## 2. Project Purpose

Wordle-DSA shows how data structures that are usually taught in theory can drive a real, playable game:

- Validate a guess against a dictionary quickly, and compare **five different lookup methods** on the same dictionary.
- Check whether a letter is part of the secret word.
- Pick secret words without repeats.
- Track the state of all 26 letters for the keyboard.
- Keep a history of guesses, and replay a finished game step by step.
- Reveal hints at a cost.
- Rank players on a persistent leaderboard.

The game is the vehicle. **The data structures are the point.**

---

## 3. Reference System

We studied the open-source console game [Loudhabs/Wordle-in-Cpp](https://github.com/Loudhabs/Wordle-in-Cpp) to understand the general Wordle architecture:

- A console game that prints `G`, `Y` or `-` for each letter.
- A word list read into a sorted vector, with guesses validated by binary search.
- A balanced BST that stores the secret word's letters for fast "is this letter in the word?" checks.
- Three classes: `Wordle` (game logic), `Tree` (balanced BST) and `Node` (tree node).

**We use it only as an architectural reference. All code in this repository is written by our team.** The reference repository has no license, so none of its code or word lists are copied here.

### Limitations we address

| Reference system | Wordle-DSA |
|------------------|------------|
| Text-only console output | Web UI with coloured tiles and an on-screen keyboard |
| BST holds only the 5 letters of the secret word | The dictionary is also stored in an **AVL tree** and a **hash table** |
| Uses STL `vector` and `sort` | Own containers (safe array, linked lists, stack, queue) and own sorting |
| Secret words can repeat | A **circular linked list** cycles through shuffled words, with no repeats |
| No record of letters already tried | 26-element letter tracker |
| No guess history | Guess history **stack** and game replay with a **doubly linked list** |
| No hints or difficulty levels | Hint **queue** and three difficulty levels |
| Nothing saved between games | Leaderboard saved to a file, with a **heap** for the top scores |

---

## 4. Changes from the Proposal

| Proposal | Current plan | Reason |
|----------|--------------|--------|
| GUI built with **Raylib** (native C++ window) | **React** web frontend + **C++** backend over HTTP | Cleaner separation of UI and DSA logic, and lets the team work on frontend and backend independently |
| BST used only for the secret word's letters | BST for the secret word's letters, plus an **AVL tree** and a **hash table** for the dictionary | Addresses the proposal's own problem statement: the BST was used on too little data |
| Data structures: BST, arrays, stack, sorted list with binary search | Extended to cover most of the course syllabus (see [Section 7](#7-course-topic-coverage)) | Apply as many of the course topics as genuinely fit the game |
| Hard mode: revealed hints must be reused | Hard mode: **5 attempts** and revealed hints must be reused | Makes Hard noticeably harder than Normal |

All the DSA work stays in C++, as the proposal describes.

---

## 5. Features

### Game
- Wordle board with a grid of tiles (5 columns, one row per attempt) that turn green, yellow or gray.
- Typing from the physical keyboard (letters, Enter, Backspace) and from an on-screen keyboard.
- An on-screen keyboard coloured by each letter's best known status.
- Random secret word with **no repeats** until every word has been used once.
- Dictionary validation: invalid words are rejected without using an attempt.
- Correct handling of **repeated letters** (see [Game Rules](#10-game-rules)).
- Three difficulty levels: Easy, Normal and Hard.
- A hint system that reveals one letter at a score penalty.
- A guess history panel.
- **Replay** of a finished game, stepping forward and backward through the guesses.

### Scores
- Score calculation, player name entry and a leaderboard saved to a file.
- Leaderboard showing the top 10 scores.
- Leaderboard search by player name.

### DSA Insights panel
- For every guess, shows how many comparisons each lookup method needed to validate the word: **linear search, binary search, interpolation search, AVL tree and hash table**.
- Shows dictionary statistics: word count, AVL tree height, hash table size, load factor and longest chain.

This turns the theory (O(n) vs O(log n) vs O(1)) into numbers the player can see.

---

## 6. Data Structures and Algorithms Used

Every structure below is **implemented by the team**. None is replaced by an STL container or algorithm.

### Containers

| Structure | Class | Used for |
|-----------|-------|----------|
| **Safe dynamic array** (1D) | `SafeArray<T>` | Bounds-checked storage for the dictionary and other lists. Throws an exception on out-of-range access instead of corrupting memory. |
| **Dynamic 2D array** | `Board` | The game grid: `rows = attempts` (8 / 6 / 5 by difficulty) × `5 letters`, with guesses and feedback |
| **Jagged array** | `LetterTracker::keyboardRows()` | Keyboard layout as 3 rows of different lengths (10, 9, 7 keys), each with its letter status |
| **Array of 26 (direct addressing)** | `LetterTracker` | Status of each letter A–Z (unused / absent / present / correct), indexed by `letter - 'A'` |
| **Singly linked list** | `SinglyLinkedList<T>` | The basis of the stack, and the hash table's collision chains |
| **Doubly linked list** | `DoublyLinkedList<T>` | Game replay: step forward and backward through the guesses |
| **Circular linked list** | `CircularLinkedList<T>` | Secret-word rotation: shuffled words in a cycle, so no word repeats until all have been used |
| **Stack** (linked-list based) | `Stack<T>` | Guess history (most recent on top), and the iterative tree traversal |
| **Queue** (linked-list based) | `Queue<T>` | Hint order: unrevealed letter positions are shuffled into a queue at game start, and each hint dequeues the next one |
| **Binary Search Tree** | `BST<T>` | The secret word's letters: O(log n) "is this letter in the word?" |
| **AVL tree** | `AVLTree<T>` | The dictionary: self-balancing tree with rotations, used for word validation |
| **Hash table** (separate chaining) | `HashTable<K, V>` | The dictionary (O(1) average validation), and active games on the server keyed by `gameId` |
| **Min-heap** (priority queue) | `MinHeap<T>` | Top-K leaderboard: streams all saved scores and keeps the best 10 |

### Algorithms

| Algorithm | Where |
|-----------|-------|
| **Linear search** | Baseline dictionary lookup, shown in the DSA Insights panel |
| **Binary search** (recursive) | Dictionary lookup on the sorted array |
| **Interpolation search** | Dictionary lookup. Each 5-letter word is mapped to a base-26 number, so the sorted words have numeric keys to interpolate on. |
| **Radix sort** (LSD) | Sorts the dictionary on load. Every word has exactly 5 letters from a 26-letter alphabet, which makes radix sort a perfect fit: O(5 × (n + 26)). |
| **Merge sort** | Sorts the top-K leaderboard entries extracted from the heap (stable, so equal scores keep their original order) |
| **Insertion sort** | Inserts one new score into the already-sorted list shown after a game |
| **Fisher–Yates shuffle** | Shuffles the secret words and the hint positions |
| **Polynomial rolling hash** | Hash function for the hash table, with **rehashing** when the load factor exceeds 0.75 |
| **Recursion** | Binary search, BST and AVL insert/search/destroy, merge sort |
| **Simulation of recursion** | Iterative in-order traversal of the AVL tree using our own stack, used to check that the tree is sorted |
| **KMP string search** | Searching the leaderboard by part of a player's name |
| **Two-pass guess evaluation** | Correct green / yellow / gray colouring with repeated letters |

### Five ways to validate one word

The same guess is checked by every method, and the comparison counts are sent to the DSA Insights panel:

```
Guess "CRANE"
     |
     ├──► Linear search ─────────── O(n)       e.g. 412 comparisons
     ├──► Binary search ─────────── O(log n)   e.g.  11 comparisons
     ├──► Interpolation search ──── O(log log n) average, e.g. 4 comparisons
     ├──► AVL tree ──────────────── O(log n)   e.g.  12 comparisons
     └──► Hash table ────────────── O(1) avg   e.g.   1 comparison
```

(The numbers are illustrative. The real ones depend on the word list.)

The **hash table** is the method the game actually relies on. The other four are run alongside it for the comparison.

### Secret word BST

```
Secret word: CRANE
Sorted letters: A C E N R

          E
        /   \
       C     R
      /     /
     A     N
```

### Secret word rotation (circular linked list)

```
   ┌──► BRAVE ──► CRANE ──► SLATE ──► PLUMB ──┐
   └──────────────────────────────────────────┘
               ↑
            current
```

Each new game moves `current` forward by one. After a full cycle, the words are reshuffled.

---

## 7. Course Topic Coverage

How each topic in the course outline is used, or why it is not.

| Week | Topic | Used? | Where |
|------|-------|-------|-------|
| 1 | ADT, C++ specification, pointers, **Rule of Three** | ✅ | Every container is an ADT with a clear interface. Every class that owns dynamic memory implements a destructor, copy constructor and copy assignment operator. |
| 2 | 1D / 2D dynamic **safe pointers**, **jagged arrays** | ✅ | `SafeArray<T>`, the 2D `Board`, the jagged keyboard rows |
| 3 | **Singly**, **doubly**, **circular** linked lists | ✅ | Stack and hash chains / game replay / secret-word rotation |
| 4 | **Recursion**, backtracking | ✅ / ❌ | Recursion: binary search, trees, merge sort. Backtracking: no natural use in Wordle, so not used. |
| 5 | **Linear**, **binary**, **interpolation** search; elementary sorts | ✅ | All three searches on the dictionary. **Insertion sort** for adding a new score. Bubble, selection and comb sort have no natural use (see stretch goal below). |
| 7 | Advanced sorts: **radix**, **merge**, quick | ✅ | Radix sort for the dictionary, merge sort for the leaderboard. Quick sort: stretch goal only. |
| 9 | **Stack**, **queue**, simulation of recursion | ✅ | Guess history stack, hint queue, iterative tree traversal |
| 10 | Binary trees, **BST** | ✅ | Secret-word BST |
| 11 | Balanced BST, **AVL tree**; 2-3 trees, B-trees | ✅ / ❌ | AVL tree for the dictionary. 2-3 trees and B-trees are designed for disk storage; the dictionary is small and in memory, so not used. |
| 12 | String search: brute force, Rabin-Karp, Boyer-Moore, **KMP** | ✅ | KMP for leaderboard name search |
| 13 | **Priority queue / heap**, **hashing**, collision resolution, **rehashing** | ✅ | Min-heap for the top-K leaderboard. Hash table with separate chaining and rehashing. |
| 14 | Graphs, MST, topological sort, shortest path | ❌ | Wordle has no network or dependency structure, so graphs would be artificial here. |

### Stretch goal: Sorting Lab

If time allows after Day 13, a **Sorting Lab** page will sort the dictionary with bubble, selection, insertion, comb, merge, quick and radix sort, and show each algorithm's comparisons, swaps and time side by side. This is optional and is only started once the core game is complete and stable.

---

## 8. System Architecture

```
┌───────────────────────────────────────────────────────────────────┐
│                     FRONTEND  (React + Vite)                      │
│                                                                   │
│  Board  Keyboard  History  Hint  Difficulty  Leaderboard  Replay  │
│                          DSA Insights                             │
│                               |                                   │
│                     api/client.js  (fetch)                        │
└───────────────────────────────┬───────────────────────────────────┘
                                │  HTTP / JSON   (localhost)
┌───────────────────────────────┴───────────────────────────────────┐
│                       BACKEND  (C++17)                            │
│                                                                   │
│                     Server (REST API)                             │
│                            |                                      │
│            HashTable<gameId, GameSession>  (active games)         │
│                            |                                      │
│                       GameSession                                 │
│   ┌──────────┬─────────────┼──────────────┬───────────┬────────┐  │
│ Wordle     Board      LetterTracker    Stack       Queue   Replay │
│   |      (2D array)   (array of 26,   (history)   (hints)  (DLL)  │
│ BST<char>              jagged rows)                               │
│                                                                   │
│  WordPicker ── CircularLinkedList (no-repeat secret words)        │
│                                                                   │
│  Dictionary ── SafeArray (radix sorted) ── linear / binary /      │
│            │                               interpolation search   │
│            ├── AVLTree<string>                                    │
│            └── HashTable<string> (chaining, rehashing)            │
│                                                                   │
│  Score ──► Leaderboard ── MinHeap (top-K) ── merge sort ── KMP    │
│                 |                                                 │
│           data/scores.txt                                         │
└───────────────────────────────────────────────────────────────────┘
```

- **The frontend only displays state and sends input.** It never sees the secret word until the game is over, and it contains no game logic.
- **The backend owns all game logic and data structures.**
- The C++ HTTP layer uses two single-header libraries kept in `backend/third_party/`: [cpp-httplib](https://github.com/yhirose/cpp-httplib) for HTTP and [nlohmann/json](https://github.com/nlohmann/json) for JSON. Neither needs a separate install, and they are used **only** in `Server.cpp`, never inside the data structures.

---

## 9. Game Flow

```
Server start
      |
      ↓
Load words.txt ──► SafeArray ──► Radix sort ──► AVL tree + Hash table
      |
      ↓
Shuffle words ──► Circular linked list
      |
      ↓
Choose difficulty ──► New game
      |
      ├──► Next secret word from the circular list
      ├──► Secret-word BST
      ├──► 2D board sized by difficulty
      ├──► Shuffled hint positions ──► Queue
      └──► GameSession stored in hash table by gameId
      |
      ↓
Player types a guess
      |
      ↓
Is it 5 letters? ── no ──► "Not enough letters"
      |
     yes
      ↓
In dictionary? ── no ──► "Not in word list" (attempt not used)
(hash table, plus all 4 other methods for DSA Insights)
      |
     yes
      ↓
Hard mode: uses every revealed hint? ── no ──► rejected (attempt not used)
      |
     yes
      ↓
Two-pass comparison with the secret word (green / yellow / gray)
      |
      ├──► Board row               (2D array)
      ├──► Letter tracker          (array of 26)
      ├──► Guess history           (stack)
      └──► Replay list             (doubly linked list)
      |
      ↓
Solved? ── yes ──► Score ──► Enter name ──► Leaderboard (file, heap, merge sort)
      |
      no
      ↓
Attempts left? ── yes ──► next guess
      |
      no
      ↓
Game over, secret word revealed, score 0, replay available
```

---

## 10. Game Rules

### Difficulty levels

| Level | Attempts | Extra rule | Score multiplier |
|-------|----------|------------|------------------|
| Easy | 8 | None | ×1.0 |
| Normal | 6 | None | ×1.5 |
| Hard | 5 | Every revealed hint must be used in later guesses | ×2.0 |

### Repeated letters

Feedback is computed in two passes so that duplicate letters are coloured correctly:

1. **First pass:** mark every letter in the correct position as **green** and use up one count of that letter from the secret word.
2. **Second pass:** for each remaining letter, mark it **yellow** if the secret word still has an unused count of that letter. Otherwise mark it **gray**.

Example, secret word `APPLE`:

| Guess | Result | Why |
|-------|--------|-----|
| `PAPER` | 🟨🟨🟩🟨⬜ | The P in position 3 is green. The first P is yellow because APPLE has a second P. E is misplaced. R is absent. |
| `PUPPY` | 🟨⬜🟩⬜⬜ | APPLE has only two P's. The middle P is green, the first P uses the remaining P and is yellow, and the extra P is gray. |

### Hints

- At the start of each game, the 5 letter positions are shuffled into a **queue**.
- A hint dequeues positions until it finds one the player has not already solved, then reveals that letter and its position.
- Each hint costs **50 points**.
- In Hard mode, a revealed letter must appear in its position in every later guess.

### Scoring

```
score = ((maxAttempts − attemptsUsed + 1) × 100 − hintsUsed × 50) × multiplier
```

- A lost game scores **0**.
- The score never goes below 0.

Example: Normal mode (6 attempts), solved on attempt 3, with 1 hint:

```
((6 − 3 + 1) × 100 − 1 × 50) × 1.5 = (400 − 50) × 1.5 = 525
```

### Leaderboard

- After a win, the player enters a name.
- The name, score, difficulty and date are appended to `data/scores.txt`.
- To show the leaderboard, all saved scores are streamed through a **min-heap of size 10**, keeping only the best 10. These are then **merge sorted** from highest to lowest.
- The player can search the leaderboard by part of a name (**KMP**).

---

## 11. Backend API (Draft)

> This is a draft. The final endpoints are fixed during the Day 9 server task and documented in `docs/API.md`.

The backend listens on `http://localhost:8080`. The Vite dev server proxies `/api` to it.

| Method | Endpoint | Body | Returns |
|--------|----------|------|---------|
| `POST` | `/api/game/new` | `{ "difficulty": "normal" }` | `gameId`, `maxAttempts`, `wordLength` |
| `POST` | `/api/game/guess` | `{ "gameId": "...", "guess": "crane" }` | Per-letter feedback, keyboard rows, attempts left, status (`playing` / `won` / `lost`), lookup stats, secret word only when the game is over |
| `POST` | `/api/game/hint` | `{ "gameId": "..." }` | Revealed letter and position, updated penalty |
| `GET` | `/api/game/history?gameId=...` | — | Guess history, most recent first (stack) |
| `GET` | `/api/game/replay?gameId=...` | — | Guesses in order, for forward/back replay (doubly linked list). Only after the game is over. |
| `GET` | `/api/leaderboard?search=...` | — | Top 10 entries (heap + merge sort), optionally filtered by name (KMP) |
| `POST` | `/api/leaderboard` | `{ "gameId": "...", "name": "Ali" }` | Saved entry and rank |
| `GET` | `/api/dsa/stats` | — | Word count, AVL height, hash table size, load factor, longest chain |

Example response to a guess:

```json
{
  "guess": "CRANE",
  "feedback": ["absent", "correct", "present", "absent", "correct"],
  "keyboard": [
    ["Q","W","E","R","T","Y","U","I","O","P"],
    ["A","S","D","F","G","H","J","K","L"],
    ["Z","X","C","V","B","N","M"]
  ],
  "letterStatus": { "C": "absent", "R": "correct", "A": "present", "N": "absent", "E": "correct" },
  "attemptsLeft": 4,
  "status": "playing",
  "lookupStats": {
    "linear": 412,
    "binary": 11,
    "interpolation": 4,
    "avl": 12,
    "hash": 1
  }
}
```

---

## 12. Project Folder Structure

```
Wordle-DSA/
│
├── backend/
│   ├── include/
│   │   ├── ds/                     # Reusable data structures (templates, header-only)
│   │   │   ├── SafeArray.h
│   │   │   ├── SinglyLinkedList.h
│   │   │   ├── DoublyLinkedList.h
│   │   │   ├── CircularLinkedList.h
│   │   │   ├── Stack.h
│   │   │   ├── Queue.h
│   │   │   ├── BST.h
│   │   │   ├── AVLTree.h
│   │   │   ├── HashTable.h
│   │   │   └── MinHeap.h
│   │   │
│   │   ├── algorithms/
│   │   │   ├── Search.h            # Linear, binary, interpolation
│   │   │   ├── Sort.h              # Radix, merge, insertion
│   │   │   ├── StringSearch.h      # KMP
│   │   │   └── Shuffle.h           # Fisher–Yates
│   │   │
│   │   ├── Dictionary.h            # Word list + all lookup methods
│   │   ├── WordPicker.h            # Circular-list secret-word rotation
│   │   ├── Board.h                 # Dynamic 2D game grid
│   │   ├── LetterTracker.h         # Array of 26 + jagged keyboard rows
│   │   ├── Wordle.h                # Secret word, BST, two-pass comparison
│   │   ├── Difficulty.h            # Easy / Normal / Hard settings
│   │   ├── GameSession.h           # One game: board, tracker, stack, queue, replay
│   │   ├── Score.h                 # Score calculation
│   │   ├── Leaderboard.h           # File, heap, merge sort, KMP search
│   │   └── Server.h                # HTTP routes
│   │
│   ├── src/
│   │   ├── main.cpp                # Starts the server only; kept small
│   │   ├── Dictionary.cpp
│   │   ├── WordPicker.cpp
│   │   ├── Board.cpp
│   │   ├── LetterTracker.cpp
│   │   ├── Wordle.cpp
│   │   ├── GameSession.cpp
│   │   ├── Score.cpp
│   │   ├── Leaderboard.cpp
│   │   └── Server.cpp
│   │
│   └── third_party/
│       ├── README.md               # Versions and licenses of the two headers
│       ├── httplib.h               # cpp-httplib 0.18.1 (single header)
│       └── json.hpp                # nlohmann/json 3.11.3 (single header)
│
├── frontend/                       # React + Vite (JavaScript)
│   ├── index.html
│   ├── package.json
│   ├── vite.config.js              # Proxies /api to localhost:8080
│   └── src/
│       ├── main.jsx
│       ├── App.jsx
│       ├── api/
│       │   └── client.js           # All fetch calls to the backend
│       ├── components/
│       │   ├── Board.jsx
│       │   ├── Tile.jsx
│       │   ├── Keyboard.jsx
│       │   ├── HistoryPanel.jsx
│       │   ├── HintButton.jsx
│       │   ├── DifficultySelect.jsx
│       │   ├── GameOverModal.jsx
│       │   ├── Replay.jsx
│       │   ├── Leaderboard.jsx
│       │   └── DsaInsights.jsx
│       └── styles/
│           └── app.css
│
├── data/
│   ├── words.txt                   # Word list (provided by the team)
│   └── scores.txt                  # Created at runtime, not committed
│
├── tests/
│   ├── TestFramework.h             # Tiny header-only test helper (no external library)
│   ├── test_safe_array.cpp
│   ├── test_linked_lists.cpp
│   ├── test_stack_queue.cpp
│   ├── test_bst.cpp
│   ├── test_avl.cpp
│   ├── test_hash_table.cpp
│   ├── test_min_heap.cpp
│   ├── test_search.cpp
│   ├── test_sort.cpp
│   ├── test_kmp.cpp
│   ├── test_dictionary.cpp
│   ├── test_board_tracker.cpp
│   ├── test_wordle.cpp
│   ├── test_game_session.cpp
│   ├── test_score.cpp
│   └── test_leaderboard.cpp
│
├── docs/
│   ├── ARCHITECTURE.md
│   ├── API.md
│   └── COMPLEXITY_ANALYSIS.md
│
├── Resources/
│   ├── Wordle_Project_Proposal.docx  # Current proposal (web frontend, extended DSA)
│   └── Wordle_Project_Proposal-1.pdf # Original proposal (Raylib version)
│
├── .gitignore
├── CMakeLists.txt
├── CONTRIBUTING.md
├── LICENSE
└── README.md
```

The generic containers and algorithms in `include/ds/` and `include/algorithms/` are **templates**. Template code has to live in the header, so these are the only exception to the "declarations in `.h`, implementations in `.cpp`" rule. All game classes keep `.h` and `.cpp` separate.

---

## 13. Development Environment

**Operating system:** Windows

| Tool | Purpose |
|------|---------|
| Git | Version control |
| Visual Studio 2022 (Desktop development with C++) **or** MinGW-w64 g++ | C++17 compiler |
| CMake 3.16 or newer | Backend build |
| Node.js 20 LTS or newer (includes npm) | Frontend tooling |
| VS Code / Dev-C++ | Editors |

Check your tools:

```bash
git --version
cmake --version
node --version
npm --version
```

---

## 14. Building and Running

> These commands work once the Day 1 project skeleton has been merged into `develop`.

### Clone

```bash
git clone https://github.com/Zidarc/Wordle-DSA.git
cd Wordle-DSA
git switch develop
```

### Backend (C++)

```bash
cmake -B build
cmake --build build
```

Run the server **from the repository root** so that it can find `data/`:

```bash
# Visual Studio (multi-config) builds
./build/Debug/wordle_server.exe

# MinGW / single-config builds
./build/wordle_server.exe
```

The server starts on `http://localhost:8080`.

### Frontend (React)

In a second terminal:

```bash
cd frontend
npm install
npm run dev
```

Open `http://localhost:5173` in a browser.

### Tests

```bash
cmake --build build
ctest --test-dir build -C Debug --output-on-failure
```

The `build/`, `node_modules/` and `frontend/dist/` directories are generated and must **not** be committed.

---

## 15. Git Branch Strategy

| Branch | Purpose |
|--------|---------|
| `main` | Stable, tested, submission-ready code |
| `develop` | Integrated team development |
| `feature/*` | New functionality, e.g. `feature/linked-lists`, `feature/avl-tree` |
| `fix/*` | Bug fixes, e.g. `fix/duplicate-letter-feedback` |
| `docs/*` | Documentation-only changes |

```
feature/your-feature ──► Pull Request ──► review ──► develop ──► (release) ──► main
```

The full workflow, Pull Request requirements and commit style are in [CONTRIBUTING.md](CONTRIBUTING.md).

---

## 16. 16-Day Schedule

**Start:** Wednesday, 4 November 2026 (Day 1)
**Submission:** Thursday, 19 November 2026 (Day 16)

The team uses a **daily rotation**. The owner of each day completes that day's task, opens a Pull Request into `develop`, and writes a handoff note for the next owner (format in [CONTRIBUTING.md](CONTRIBUTING.md#10-daily-handoff)).

Rotation order: **Ali Hussain → Ali → M. Abbas**, repeating.

| Day | Date | Owner | Task | Branch |
|-----|------|-------|------|--------|
| 1 | Wed 4 Nov | Ali Hussain | Docs, `.gitignore`, folder structure, CMake + CTest skeleton, Vite skeleton, vendor `httplib.h` / `json.hpp`, create `develop`. `SafeArray<T>` (Rule of Three, bounds checking) + tests | `feature/project-setup` |
| 2 | Thu 5 Nov | Ali | `SinglyLinkedList`, `DoublyLinkedList`, `CircularLinkedList`, `Stack`, `Queue` (all Rule of Three) + tests | `feature/linked-lists` |
| 3 | Fri 6 Nov | M. Abbas | `BST<T>`, `AVLTree<T>` (insert, rotations, search, height, iterative in-order with `Stack`) + tests | `feature/trees` |
| 4 | Sat 7 Nov | Ali Hussain | `HashTable<K,V>` (polynomial hash, chaining, rehashing, stats) and `MinHeap<T>` + tests | `feature/hash-heap` |
| 5 | Sun 8 Nov | Ali | Algorithms: linear / binary / interpolation search with comparison counters, radix / merge / insertion sort, KMP, Fisher–Yates + tests | `feature/algorithms` |
| 6 | Mon 9 Nov | M. Abbas | `Dictionary` (load, radix sort, AVL, hash table, 5-way lookup stats) and `WordPicker` (circular list) + tests | `feature/dictionary` |
| 7 | Tue 10 Nov | Ali Hussain | `Board` (2D), `LetterTracker` (26 array + jagged rows), `Wordle` (BST, two-pass comparison) + tests | `feature/wordle-core` |
| 8 | Wed 11 Nov | Ali | `Difficulty`, `GameSession` (history stack, hint queue, Hard-mode rule, replay list), `Score`, `Leaderboard` (file, heap, merge sort, KMP) + tests | `feature/session-leaderboard` |
| 9 | Thu 12 Nov | M. Abbas | `Server`: HTTP routes, games in `HashTable`, JSON, CORS, `docs/API.md` | `feature/server-api` |
| 10 | Fri 13 Nov | Ali Hussain | Frontend: `api/client.js`, `Board`, `Tile`, physical keyboard input, new game + guess flow | `feature/frontend-board` |
| 11 | Sat 14 Nov | Ali | Frontend: on-screen `Keyboard`, `HistoryPanel`, `HintButton`, `DifficultySelect` | `feature/frontend-controls` |
| 12 | Sun 15 Nov | M. Abbas | Frontend: `GameOverModal`, name entry, `Leaderboard` with search, `Replay` | `feature/frontend-leaderboard` |
| 13 | Mon 16 Nov | Ali Hussain | Frontend: `DsaInsights` panel. End-to-end integration and error handling (invalid word, server offline) | `feature/integration` |
| 14 | Tue 17 Nov | Ali | Full test pass, bug fixes, `docs/COMPLEXITY_ANALYSIS.md` | `fix/*` / `docs/complexity` |
| 15 | Wed 18 Nov | M. Abbas | UI polish, `docs/ARCHITECTURE.md`, demo script and rehearsal. *Stretch: Sorting Lab* | `feature/ui-polish` / `docs/*` |
| 16 | Thu 19 Nov | Ali Hussain | Final regression test, merge `develop` → `main`, tag `v1.0`, submit | `main` |

### Build order

The order is deliberate. Each step depends on the one before it.

```
1. Safe array
   ↓
2. Linked lists → Stack, Queue
   ↓
3. BST, AVL tree            (AVL traversal uses Stack)
   ↓
4. Hash table, Min-heap     (chains use Singly linked list)
   ↓
5. Search / sort / KMP / shuffle algorithms
   ↓
6. Dictionary + WordPicker  (uses 1, 2, 3, 4, 5)
   ↓
7. Board, Letter tracker, Wordle
   ↓
8. GameSession, Score, Leaderboard
   ↓
9. HTTP server
   ↓
10. React frontend
   ↓
11. Integration, testing, documentation, release
```

If a day's task is not finished, the owner states this in the handoff, and the next owner finishes it before starting their own task. **A broken feature is never merged just to stay on schedule.**

---

## 17. Testing Requirements

Every backend component has its own test file in `tests/`, and the tests run through CTest.

**Safe array**
- Valid access; out-of-range access throws
- Copy constructor and assignment make a deep copy (Rule of Three)
- Resize keeps the existing elements

**Linked lists, Stack, Queue**
- Empty list / stack / queue
- Insert and remove at the head, tail and middle (singly and doubly)
- Doubly linked list: forward and backward traversal
- Circular linked list: wraps around after the last node
- Stack: LIFO order, pop / peek on empty
- Queue: FIFO order, dequeue on empty
- Deep copy for each (Rule of Three)

**BST and AVL tree**
- Empty tree, single element
- Search: present and missing values
- AVL: each of the four rotations (LL, RR, LR, RL)
- AVL: height stays O(log n) after inserting sorted data
- Iterative in-order traversal returns sorted output

**Hash table and Min-heap**
- Insert, search, missing key
- Duplicate key
- Collisions land in the same chain
- Rehashing keeps every entry findable
- Heap: insert, extract-min order, top-K of a larger list

**Algorithms**
- Linear, binary and interpolation search: first, middle, last, missing; all three agree
- Radix, merge and insertion sort: empty, one element, already sorted, reversed, duplicates
- KMP: match at the start / middle / end, no match, overlapping matches

**Dictionary and WordPicker**
- Load the word list; missing or empty file
- All five lookup methods give the same answer for every test word
- Case-insensitive input
- WordPicker: no repeats within one full cycle

**Board, Letter tracker, Wordle**
- Board size matches the difficulty (8 / 6 / 5 rows)
- Letter status only upgrades (absent → present → correct, never downgrades)
- Keyboard rows have lengths 10, 9 and 7
- All-green guess (win), all-gray guess, misplaced letters
- Repeated letters in the guess (`PUPPY` vs `APPLE`) and in the secret word
- Running out of attempts (loss)

**GameSession, Score, Leaderboard**
- History stack holds the guesses, most recent first
- Hint queue never reveals an already-solved position
- Hard mode rejects a guess that ignores a revealed hint
- Replay list walks forward and backward
- Each difficulty multiplier; hint penalty; loss gives 0; never below 0
- Leaderboard save and reload; missing `scores.txt`; top 10 sorted highest first; equal scores; name search

---

## 18. Complexity Overview

`n` = words in the dictionary, `L` = word length (5), `k` = guesses made, `m` = saved scores, `K` = leaderboard size (10), `p` = search pattern length.

| Operation | Structure / Algorithm | Time |
|-----------|-----------------------|------|
| Sort dictionary on load | Radix sort | O(L × (n + 26)) |
| Build AVL tree | AVL insert × n | O(n log n) |
| Build hash table | Hash insert × n | O(n) average |
| Validate word | Linear search | O(n) |
| Validate word | Binary search | O(log n) |
| Validate word | Interpolation search | O(log log n) average, O(n) worst |
| Validate word | AVL tree | O(log n) |
| Validate word | Hash table | O(1) average, O(n) worst |
| Next secret word | Circular linked list | O(1) |
| Letter in secret word? | BST | O(log L) |
| Evaluate a guess (two passes) | Arrays + BST | O(L log L) |
| Update / read a letter's status | Array of 26 | O(1) |
| Push / pop guess | Stack | O(1) |
| Next hint | Queue | O(1) amortised |
| Replay step | Doubly linked list | O(1) |
| Top-K leaderboard | Min-heap of size K | O(m log K) |
| Sort top-K | Merge sort | O(K log K) |
| Search leaderboard by name | KMP | O(m × (name length + p)) |
| Find active game | Hash table | O(1) average |

The full analysis will be in `docs/COMPLEXITY_ANALYSIS.md`.

---

## 19. Final Demonstration

The demo walks through one complete game and then the supporting features.

1. Start the backend. Show the dictionary loading: **radix sort**, then the **AVL tree** and **hash table** being built (`/api/dsa/stats`).
2. Select **Normal** difficulty and start a new game. The secret word comes from the **circular linked list**.
3. Type an invalid word, e.g. `XQZTV`. It is rejected as "Not in word list", and no attempt is used.
4. Make a valid guess and open **DSA Insights**: compare the comparison counts of linear, binary, interpolation, AVL and hash table lookups.
5. Show the coloured tiles (**2D board**) and the on-screen keyboard (**array of 26**, **jagged** rows) updating.
6. Make a guess with **repeated letters** and explain the two-pass colouring.
7. Use a **hint** (**queue**) and show the 50-point penalty.
8. Open the **guess history** (**stack**, most recent on top).
9. Solve the word, show the score, enter a name and see the **leaderboard** (**min-heap** top 10, **merge sort**).
10. Search the leaderboard by part of a name (**KMP**).
11. **Replay** the finished game forward and backward (**doubly linked list**).
12. Restart the server and show that the leaderboard persists (file handling).
13. Start a **Hard** game and show the 5-attempt limit and the hint-reuse rule.
14. Walk through the course topic table (Section 7) and the complexity table (Section 18).

---

## 20. Out of Scope

The following are intentionally **not** part of this project:

- Online multiplayer
- User accounts, logins or authentication
- Databases (text files only)
- Online deployment / hosting
- Mobile applications
- AI / automatic solver
- A daily shared word synced with the official game
- External word or dictionary APIs
- Graphs, 2-3 trees and B-trees, which have no natural fit in this game (see Section 7)

Major scope changes need agreement from the whole team and the instructor.

---

## 21. Learning Outcomes

By completing this project, the team will have practised:

- ADT design and the Rule of Three
- Safe dynamic arrays, dynamic 2D arrays and jagged arrays
- Singly, doubly and circular linked lists
- Stacks and queues built on linked lists
- Recursion, and simulating recursion with an explicit stack
- Linear, binary and interpolation search, and comparing them in practice
- Radix, merge and insertion sort
- Binary Search Trees and AVL trees with rotations
- Hash tables, hash functions, collision resolution and rehashing
- Heaps as priority queues (top-K selection)
- KMP string search
- File handling for persistent data
- Generic programming with C++ templates
- CMake builds and automated tests with CTest
- A REST API in C++, and a React frontend talking to it
- Git branching, Pull Requests and code review
- Big-O analysis

---

## 22. Credits and License

- Architecture reference: [Loudhabs/Wordle-in-Cpp](https://github.com/Loudhabs/Wordle-in-Cpp). Used for the general design only; no code was copied.
- Original game concept: *Wordle* by Josh Wardle. This is an educational project and is not affiliated with or endorsed by the original game or its owners.
- [cpp-httplib](https://github.com/yhirose/cpp-httplib) (MIT) and [nlohmann/json](https://github.com/nlohmann/json) (MIT).

This project is licensed under the MIT License. See [LICENSE](LICENSE).
