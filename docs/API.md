# Wordle-DSA Backend REST API

This document describes the HTTP REST API for the Wordle-DSA backend running on `http://localhost:8080`.

The backend is built in C++17 using `httplib.h` and `json.hpp`. All data structures (AVL tree, hash tables, heaps, stacks, queues, linked lists, safe arrays) and search/sort algorithms are hand-implemented.

---

## Base URL & Configuration

* **Default Base URL**: `http://localhost:8080`
* **Vite Dev Proxy**: Requests from the frontend to `/api/*` are forwarded to `http://localhost:8080/api/*`.
* **CORS**: Enabled on all endpoints (`Access-Control-Allow-Origin: *`, `Access-Control-Allow-Methods: GET, POST, OPTIONS`).

---

## Endpoints

### 1. `POST /api/game/new`
Starts a new game session.

#### Request
* **Body** (optional JSON):
  ```json
  {
    "difficulty": "normal"
  }
  ```
  Supported difficulties: `"easy"`, `"normal"`, `"hard"`. Defaults to `"normal"`.

#### Response (`200 OK`)
```json
{
  "gameId": "game-1-123456",
  "difficulty": "normal",
  "maxAttempts": 6,
  "wordLength": 5
}
```

#### Underlying DSA Operations
* Generates a unique game ID.
* Calls `WordPicker::nextWord()` to get the next secret word from the `CircularLinkedList` (zero repeats).
* Instantiates `GameSession` and stores it into `HashTable<std::string, GameSession>`.

---

### 2. `POST /api/game/guess`
Submits a 5-letter word guess for validation and evaluation.

#### Request
* **Body** (JSON):
  ```json
  {
    "gameId": "game-1-123456",
    "guess": "CRANE"
  }
  ```

#### Successful Response (`200 OK`)
```json
{
  "guess": "CRANE",
  "valid": true,
  "feedback": ["absent", "correct", "present", "absent", "correct"],
  "keyboard": [
    ["Q","W","E","R","T","Y","U","I","O","P"],
    ["A","S","D","F","G","H","J","K","L"],
    ["Z","X","C","V","B","N","M"]
  ],
  "letterStatus": {
    "C": "absent",
    "R": "correct",
    "A": "present",
    "N": "absent",
    "E": "correct"
  },
  "attemptsLeft": 4,
  "attemptsUsed": 2,
  "status": "playing",
  "score": 0,
  "secretWord": "",
  "lookupStats": {
    "linear": 412,
    "binary": 11,
    "interpolation": 4,
    "avl": 12,
    "hash": 1
  }
}
```
*Note: `secretWord` is kept empty (`""`) while `status` is `"playing"`. It is only revealed when `status` is `"won"` or `"lost"`.*

#### Invalid Word Response (`400 Bad Request`)
If the guess is not 5 letters, not in the dictionary, or violates the Hard-mode hint-reuse rule:
```json
{
  "guess": "XYZWQ",
  "valid": false,
  "error": "Not in word list",
  "attemptsLeft": 4,
  "attemptsUsed": 2,
  "status": "playing",
  "score": 0,
  "lookupStats": {
    "linear": 701,
    "binary": 11,
    "interpolation": 2,
    "avl": 12,
    "hash": 1
  }
}
```
*(An invalid guess does not consume an attempt).*

#### Underlying DSA Operations
* Evaluates word presence using `Dictionary::lookupAll()` across all 5 lookup methods (Linear search, Binary search, Interpolation search, AVL tree search, Hash table search).
* In Hard mode, checks hint-reuse constraints.
* Runs two-pass comparison using the secret-word's `BST<char>` to correctly colour duplicate letters.
* Updates 2D dynamic `Board`, letter tracker array of 26, guess history `Stack`, and replay `DoublyLinkedList`.

---

### 3. `POST /api/game/hint`
Reveals one unsolved letter position at a 50-point score penalty.

#### Request
* **Body** (JSON):
  ```json
  {
    "gameId": "game-1-123456"
  }
  ```

#### Response (`200 OK`)
```json
{
  "position": 2,
  "letter": "A",
  "hintsUsed": 1,
  "penalty": 50,
  "message": "Hint revealed"
}
```

#### Underlying DSA Operations
* Dequeues positions from `Queue<int> hintQueue_` (shuffled with Fisher-Yates at game start) until an unsolved letter position is found.

---

### 4. `GET /api/game/history?gameId={gameId}`
Fetches the guess history for the specified game session.

#### Response (`200 OK`)
```json
{
  "gameId": "game-1-123456",
  "history": ["SLATE", "CRANE"]
}
```
*Guessed words are returned in LIFO order (most recent first) from the `Stack<std::string>`.*

---

### 5. `GET /api/game/replay?gameId={gameId}`
Steps through a completed game in forward or reverse chronological order.

#### Response (`200 OK`)
```json
{
  "gameId": "game-1-123456",
  "secretWord": "APPLE",
  "status": "won",
  "score": 525,
  "steps": [
    {
      "step": 1,
      "guess": "CRANE",
      "feedback": ["absent", "absent", "present", "absent", "correct"]
    },
    {
      "step": 2,
      "guess": "APPLE",
      "feedback": ["correct", "correct", "correct", "correct", "correct"]
    }
  ]
}
```
*Only available after game over. Iterates the game's `DoublyLinkedList<GuessFeedback>`.*

---

### 6. `GET /api/leaderboard?search={query}`
Retrieves the top 10 scores, optionally filtered by a player's name.

#### Response (`200 OK`)
```json
[
  {
    "rank": 1,
    "name": "Ali Hussain",
    "score": 600,
    "difficulty": "normal",
    "date": "2026-11-19"
  },
  {
    "rank": 2,
    "name": "Abbas",
    "score": 525,
    "difficulty": "normal",
    "date": "2026-11-19"
  }
]
```

#### Underlying DSA Operations
* Streams all saved scores from `data/scores.txt` through a `MinHeap` of size 10 (`pushBounded`).
* Extracts entries and sorts them in descending order using **Merge Sort**.
* If `search` parameter is present, filters names using **KMP String Search**.

---

### 7. `POST /api/leaderboard`
Submits a winning player's score to the leaderboard.

#### Request
* **Body** (JSON):
  ```json
  {
    "gameId": "game-1-123456",
    "name": "Abbas"
  }
  ```

#### Response (`200 OK`)
```json
{
  "saved": true,
  "name": "Abbas",
  "score": 525,
  "difficulty": "normal",
  "date": "2026-11-19"
}
```

---

### 8. `GET /api/dsa/stats`
Returns live structural metrics of the dictionary data structures for the DSA Insights panel.

#### Response (`200 OK`)
```json
{
  "wordCount": 701,
  "avlHeight": 11,
  "hashSize": 701,
  "hashBuckets": 1009,
  "hashUsedBuckets": 512,
  "hashLongestChain": 4,
  "hashRehashCount": 7,
  "hashLoadFactor": 0.69,
  "avlLeaves": 351,
  "avlBalanced": true,
  "btreeHeight": 9,
  "btreeNodes": 695
}
```

---

## Phase 2 endpoints

The guess response (`POST /api/game/guess`) also gained three fields:

| Field | Meaning |
|-------|---------|
| `lookupStats.btree` | Comparisons made by the sixth lookup method, the 2-3 tree |
| `btreeNodesVisited` | B-tree nodes read on the way down (each holds 1-2 words) |
| `possibleWords` | Dictionary words that still fit every guess so far (solver's linked list) |

### 9. `GET /api/game/solver?gameId={gameId}&count={n}`
Rebuilds the solver from the game's replay list and runs the backtracking search twice: recursively and with an explicit `ArrayStack`. Both must report the same numbers. `suggestions` (ranked by letter frequency with quick sort) stays empty until 2 guesses have been made, so it cannot be used to skip the game. `count` defaults to 5.

#### Response (`200 OK`)
```json
{
  "gameId": "game-1-723400",
  "possibleWords": 4,
  "backtracking":          { "nodesVisited": 1040, "pruned": 997, "prefixChecks": 1544, "maxDepth": 5, "solutions": 4 },
  "backtrackingIterative": { "nodesVisited": 1040, "pruned": 997, "prefixChecks": 1544, "maxDepth": 5, "solutions": 4 },
  "suggestions": [ { "word": "BLAME", "score": 34 }, { "word": "BLADE", "score": 32 } ]
}
```

### 10. `GET /api/dsa/sort?limit={n}`
Sorting Lab. Sorts the same shuffled copy of the dictionary with bubble, selection, insertion, comb, merge, quick and radix sort. `limit` (optional) sorts only the first `n` words.

#### Response (`200 OK`)
```json
{
  "inputSize": 701,
  "runs": [
    { "algorithm": "bubble", "complexity": "O(n^2)", "comparisons": 245305, "swaps": 119495, "microseconds": 20423, "sortedCorrectly": true },
    { "algorithm": "radix",  "complexity": "O(L(n + 26))", "comparisons": 0, "swaps": 3505, "microseconds": 410, "sortedCorrectly": true }
  ]
}
```

### 11. `GET /api/dsa/trees`
Binary tree properties of the dictionary's AVL tree, and the shape of the 2-3 tree.

#### Response (`200 OK`)
```json
{
  "avl": { "nodes": 701, "leaves": 351, "internalNodes": 350, "singleChildNodes": 0, "height": 10,
           "minPossibleHeight": 10, "full": true, "complete": false, "perfect": false },
  "btree": { "order": 3, "keys": 701, "nodes": 695, "height": 9, "valid": true }
}
```

### 12. `GET /api/games/recent`
The last 10 finished games, most recent first. Stored in a bounded circular `ArrayQueue`, which drops the oldest game when full.

#### Response (`200 OK`)
```json
[
  { "gameId": "game-3-118204", "secretWord": "CRANE", "difficulty": "normal", "result": "won", "attempts": 4, "score": 450 }
]
```

### 13. Choosing the lookup method

`POST /api/game/guess` accepts an optional `"method"`: `linear`, `binary`, `interpolation`, `avl`, `btree` or `hash` (default). That method decides whether the word is in the list. All six still run, and the reply adds:

| Field | Meaning |
|-------|---------|
| `methodUsed` | The method that decided |
| `lookupMethods` | One row per method: `key`, `name`, `structure`, `found`, `comparisons`, `nanoseconds` (average of 200 runs), `timeBest`, `timeAvg`, `timeWorst`, `spaceStructure`, `spaceExtra`, `used` |

### 14. `GET /api/dsa/lookup?word={word}&method={method}`
Looks any 5-letter word up with all six methods, without a game. Same `methods` rows as above.

#### Response (`200 OK`, shortened)
```json
{
  "word": "ZEBRA",
  "found": true,
  "methodUsed": "avl",
  "wordCount": 701,
  "timingRepeats": 200,
  "methods": [
    { "key": "linear", "comparisons": 701, "nanoseconds": 32344, "timeAvg": "O(n)", "spaceExtra": "O(1)", "used": false },
    { "key": "avl", "comparisons": 20, "nanoseconds": 734, "timeAvg": "O(log n)", "spaceExtra": "O(log n) recursion stack", "used": true },
    { "key": "hash", "comparisons": 1, "nanoseconds": 143, "timeAvg": "O(1)", "spaceExtra": "O(1)", "used": false }
  ]
}
```
