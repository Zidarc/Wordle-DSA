# Part 1: Data Structures

Every container the game uses is written by the team in `backend/include/ds/`. None is replaced by an STL container. This part goes through them in the order they were built, because each one is used by the next.

```
SafeArray ──► MinHeap (heap stored in a SafeArray)
          └─► Dictionary's sorted word list

SinglyLinkedList ──► Stack, Queue
                 ├─► HashTable chains
                 └─► Solver's candidate list

Stack ──► AVL iterative traversal       Queue ──► hint order, isComplete()
ArrayStack ──► iterative quick sort, iterative backtracking
ArrayQueue ──► recent games ring buffer
```

---

## 1.1 ADTs and the Rule of Three

**ADT (abstract data type):** a type defined by *what it does* (push, pop, contains...) rather than how. Every class in `ds/` is an ADT: the game code calls `Stack::push`, never touches the nodes.

**Rule of Three:** if a class owns memory through a raw pointer (`new`), the compiler's default copy only copies the pointer. Two objects would then share one block, and both destructors would `delete` it (double free). So the class must define all three:

| Member | Job |
|--------|-----|
| Destructor `~X()` | `delete` what it owns |
| Copy constructor `X(const X&)` | Allocate new memory and copy the *contents* (deep copy) |
| Copy assignment `operator=` | Free the old memory, deep copy, and handle `a = a` |

**Where to see it:** `SafeArray.h`, `SinglyLinkedList.h`, `BTree.h`, `Board.cpp`, `LetterTracker.cpp`, `FrequencyTable.cpp`.

**Copy-and-swap / copy-first pattern.** Most `operator=` in this project first build the copy, *then* release the old data:

```cpp
SinglyLinkedList temp(other); // may throw: *this is still untouched
clear();                       // only now drop the old nodes
// steal temp's nodes ...
```

If `new` throws half way, the object is left as it was. This is called the *strong exception guarantee*.

**Classes that do not need it:** `Stack`, `Queue` and `MinHeap` own no raw pointer; their only member (a list or a `SafeArray`) already copies itself correctly, so the defaults are right. They still write `= default` to show it is deliberate.

---

## 1.2 SafeArray — `ds/SafeArray.h`

**What:** a growable array (like `std::vector`) whose `operator[]` checks the index and throws `std::out_of_range` instead of reading random memory.

**Layout:** `data_` (heap array), `size_` (used), `capacity_` (allocated).

**pushBack and growth:** when `size_ == capacity_`, a new array of double the capacity is allocated and the elements are copied over. One push can cost O(n), but doubling means n pushes cost O(n) in total, so each push is **O(1) amortised**.

| Operation | Time |
|-----------|------|
| `operator[]`, `at` | O(1) + bounds check |
| `pushBack` | O(1) amortised |
| `insertAt`, `removeAt` | O(n) (shifting) |
| copy | O(n) |

**Used by:** the dictionary's sorted word list, `MinHeap`'s storage, every function that returns a list (`SortingLab::runAll`, `Solver::suggestions` ...).

**Why not a plain array?** The dictionary size is only known after reading the file, and the bounds check turns an off-by-one into a clear exception during testing.

---

## 1.3 Linked lists — `SinglyLinkedList.h`, `DoublyLinkedList.h`, `CircularLinkedList.h`

### Singly linked list
Each node holds `data` and `next`. The list keeps `head_`, `tail_` and `size_`.

| Operation | Time | Why |
|-----------|------|-----|
| `pushFront`, `popFront` | O(1) | only the head changes |
| `pushBack` | O(1) | thanks to `tail_` |
| `popBack` | O(n) | must walk to the node *before* the tail |
| `at(i)`, `insertAt`, `removeAt` | O(n) | walk i steps |
| `removeIf` (first match) | O(n) | |
| `removeAllIf` | O(n) | one pass with a "node before" pointer |

`removeAllIf` is used by the solver: after a guess, every impossible word is unlinked in one pass. Removing from the middle of a linked list is just pointer surgery (`before->next = doomed->next`), no shifting like an array.

**Used by:** `Stack`, `Queue`, the hash table chains, the solver's candidate list.

### Doubly linked list
Each node also has `prev`, so the list can be walked **backwards**, and removing a node you already hold is O(1).

**Used by:** game replay (`GameSession::replay_`). The replay endpoint walks forwards; the frontend steps back and forth through the same order.

### Circular linked list
The last node's `next` points back to the first, and the class keeps a **cursor**. `advance()` moves the cursor one step; it never "falls off" the end.

**Used by:** `WordPicker`, the secret word rotation (§3.2). The words are shuffled into the circle; each new game takes `current()` and `advance()`s. After one full lap every word has been used once, so no word repeats until all have been played.

---

## 1.4 Stacks and queues — two implementation strategies

| | Linked (`Stack.h`, `Queue.h`) | Array (`ArrayStack.h`, `ArrayQueue.h`) |
|---|---|---|
| Storage | `SinglyLinkedList` | dynamic array |
| push / enqueue | O(1), one `new` | O(1) amortised, may double the array |
| pop / dequeue | O(1), one `delete` | O(1), nothing freed |
| Memory | one pointer per element | spare capacity at the end |
| Locality | nodes scattered in memory | elements side by side (cache friendly) |

### Stack (LIFO)
- **Linked `Stack`:** the top is the list's **head** (push = `pushFront`, pop = `popFront`). Using the tail would make pop O(n), because a singly linked list cannot step back.
- **`ArrayStack`:** the top is `data_[size_-1]`.
- **Used by:** guess history (`GameSession::history_`, newest on top); the iterative AVL traversal; iterative quick sort and iterative backtracking (`ArrayStack`).

### Queue (FIFO)
- **Linked `Queue`:** enqueue at the tail (`pushBack`), dequeue at the head (`popFront`), both O(1).
- **`ArrayQueue` (circular array):** a plain array queue would shift everything on each dequeue. Instead `front_` moves forward and positions wrap with `% capacity_`:

```
capacity 6, size 4, front_ = 4
index:  0    1    2    3    4    5
      [ C ][ D ][   ][   ][ A ][ B ]      removal order: A, B, C, D
```

  - *Growing mode:* when full, copy into a double-size array in queue order (this undoes the wrap).
  - *Bounded mode:* when full, drop the oldest element. This is a **ring buffer**.
- **Used by:** hint order (`GameSession::hintQueue_`, linked); recent games (`Server::recentGames_`, bounded `ArrayQueue` of 10); level-order walk in `btIsComplete`.

---

## 1.5 Binary Search Tree — `ds/BST.h`

**Rule:** for every node, everything in the left subtree is smaller and everything in the right subtree is larger.

**Search:** compare with the node, go left or right, repeat → O(h) where h is the height.

**Problem:** the height depends on the insertion order. Inserting sorted data makes a "linked list in disguise" (h = n). The test `sortedInsertDegeneratesBstButNotAvl` shows a height of 63 for 63 values.

**Used by:** the secret word's letters (`Wordle::letters_`). `Wordle`'s constructor sorts the 5 letters and inserts them middle-first (order 2, 1, 4, 0, 3) so this tiny BST is balanced:

```
Secret CRANE → sorted A C E N R →        E
                                        / \
                                       C   R
                                      /   /
                                     A   N
```

`containsLetter()` answers "is this letter anywhere in the word?" for the yellow pass of `evaluate` (§3.3).

**Recursive helpers:** `insert(Node*, value)`, `contains(...)`, `height(...)`, `destroy(...)`, `copyTree(...)`. `destroy` is **head-style recursion**: free the children first, then the node.

---

## 1.6 AVL tree — `ds/AVLTree.h`

**What:** a BST that rebalances after every insert, so the height stays O(log n) no matter the insert order.

**Balance factor** = height(left) − height(right). AVL keeps it in {−1, 0, +1} for every node. Each node stores its own `height` so this is O(1) to compute.

**The four cases** (after inserting, walking back up, at the first unbalanced node):

| Case | Shape | Fix |
|------|-------|-----|
| LL | left child's left grew | rotate right |
| RR | right child's right grew | rotate left |
| LR | left child's right grew | rotate left on the child, then right |
| RL | right child's left grew | rotate right on the child, then left |

```
LL: rotate right at z          z            y
                              /            / \
                             y     →      x   z
                            /
                           x
```

All four are handled in `balance(Node*)`, called on the way back out of the recursive `insert`.

**Iterative in-order traversal (`toSortedArray`):** uses our own `Stack<const Node*>` instead of recursion — "go left as far as possible pushing nodes; pop, visit, go right". This is **simulation of recursion** (§2.5). `Dictionary::treeMatchesSortedWords()` uses it to prove the tree is still sorted.

**`isBalanced()`** re-checks the AVL rule and every stored height recursively; the tests run it after 2000 inserts.

**Used by:** the dictionary (lookup method `avl`). 701 words → height 10, the minimum possible for 701 nodes.

| Operation | Time |
|-----------|------|
| insert / contains | O(log n) guaranteed |
| height | O(1) (stored) |
| toSortedArray, copy, destroy | O(n) |

---

## 1.7 Binary tree properties — `ds/BinaryTreeProperties.h`

Free template functions that take a root pointer, used by both `BST` and `AVLTree` (`isFull()`, `isComplete()`, `isPerfect()`, `leafCount()`, `shape()`).

| Property | Definition | How it is checked |
|----------|-----------|-------------------|
| **Full** | every node has 0 or 2 children | recursion: no node with exactly one child |
| **Complete** | all levels full except maybe the last, which fills from the left | level-order walk with `Queue`: after the first missing child, no further node may have a child |
| **Perfect** | full and all leaves on one level | `n == 2^h − 1` |

Useful facts the tests check:
- A full binary tree has `leaves = internal + 1` (the dictionary's AVL tree: 351 leaves, 350 internal).
- A heap array is always a complete tree (that is why `MinHeap` can use `parent = (i−1)/2`).
- `minPossibleHeight = ceil(log2(n + 1))`; the AVL tree reaches it for 701 words (10).

**Shown in:** `GET /api/dsa/trees` → DSA Lab panel.

---

## 1.8 B-tree of order 3 (2-3 tree) — `ds/BTree.h`

**What:** a search tree whose nodes hold **up to Order−1 keys** and **up to Order children**. With Order = 3 every internal node has 2 or 3 children and 1 or 2 keys: a **2-3 tree**.

```
                [ GHOST | PLUMB ]
               /        |        \
      [ APPLE ]   [ LEMON ]   [ STONE | ZEBRA ]
```

**Rules kept after every insert:** keys in a node are sorted; non-root nodes have at least ⌈Order/2⌉−1 keys; **all leaves are on the same level**.

**Insert (bottom-up split)** — `insert` → `insertInto` → `splitChild`:
1. Walk down to the right leaf and insert the key there in order.
2. If the node now has Order keys (one too many), **split**: the middle key moves up to the parent, the left and right halves become two nodes.
3. The parent may now overflow → split again on the way up.
4. If the root splits, a new root is created above it: **the tree only grows at the top**, which is why all leaves stay level.

The `Node` arrays have one spare slot (`keys[Order]`) so a node can hold the extra key for a moment before splitting.

**Search:** inside a node, scan the keys left to right until `key < keys[i]`, then go down `children[i]`. `search()` counts comparisons and nodes visited.

**Why it is here:** lab week 11, and to compare with the AVL tree. Fewer, wider nodes mean a shorter tree. B-trees are used by databases and file systems because each node fits one disk block, so fewer levels = fewer disk reads.

**Honest result on this word list:** the words are inserted already sorted. Bottom-up splitting then leaves most nodes with one key (695 nodes for 701 keys), so the 2-3 tree is only one level shorter than the AVL tree (9 vs 10). `test_btree` shows that higher orders (`BTree<int, 5>`, `BTree<int, 16>`) give clearly shorter trees.

**`isValid()`** checks every rule recursively, including that child keys lie between their separators.

---

## 1.9 Hash table — `ds/HashTable.h`

**What:** key → value map with O(1) average lookup.

**Layout:** an array of buckets; each bucket is a `SinglyLinkedList<Entry>` (**separate chaining**). Keys whose hash lands in the same bucket share the chain.

**Hash function** (`KeyHash<std::string>`): polynomial rolling hash with Horner's rule,

```
h = s[0]·31^(n−1) + s[1]·31^(n−2) + … + s[n−1]      (mod 2^64, by unsigned overflow)
bucket = h % bucketCount
```

**Rehashing:** when `size / bucketCount > 0.75` after an insert, the table grows to the **next prime ≥ 2 × buckets** and every entry is re-inserted (`rehash`). Prime bucket counts spread keys more evenly. Loading 701 words causes several rehashes (shown as "Rehashes" in DSA Insights).

| Operation | Average | Worst |
|-----------|---------|-------|
| insert / contains / find / remove | O(1) | O(n) (everything in one chain) |
| rehash | O(n + b) | |

**Comparisons in DSA Insights** = how many chain entries were compared; usually 1.

**Used by:** the dictionary (lookup method `hash`, also the duplicate filter while loading) and the server's active games `HashTable<std::string, GameSession> sessions_`.

---

## 1.10 Min-heap — `ds/MinHeap.h`

**What:** a priority queue stored as a **complete binary tree in an array** (`SafeArray`):

```
parent(i) = (i − 1) / 2     left(i) = 2i + 1     right(i) = 2i + 2
```

**Heap property:** every parent ≤ its children, so the minimum is at index 0.

- `push`: append at the end, **sift up** while smaller than the parent — O(log n).
- `pop`: move the last element to the root, **sift down** to the smaller child — O(log n).
- `MinHeap(const SafeArray&)`: build in O(n) by sifting down from the last parent (heapify).
- `pushBounded(value, K)`: keep only the K largest seen so far. If the heap is full and `value` beats the root (the smallest of the K), it replaces the root.

**Top-K trick (leaderboard):** to find the best 10 of m scores you do not sort all m. Stream them through a heap of size 10: O(m log 10) instead of O(m log m). Then sort just those 10 (merge sort). See §3.6.

---

## 1.11 2D and jagged arrays — `Board`, `LetterTracker`, `FrequencyTable`

### Board (dynamic 2D array) — `Board.h/.cpp`
`char** letters_` and `LetterState** states_`: an array of row pointers, each row an array of 5. Rows = attempts for the difficulty (8 / 6 / 5). `addRow` writes the next guess. Rule of Three is implemented by hand (`allocate`, `release`, `copyCellsFrom`).

### LetterTracker (direct addressing + jagged array) — `LetterTracker.h/.cpp`
- `status_[26]`: one slot per letter, indexed by `letter − 'A'` (**direct addressing**, O(1)). A letter's state only upgrades: unused → absent → present → correct.
- `keyRows_`: a **jagged array**, 3 rows of different lengths (10, 9, 7) for the keyboard layout.

### FrequencyTable (5 × 26) — `FrequencyTable.h/.cpp`
`counts_[position][letter]` counts how many possible words have each letter at each position; `totals_[letter]` counts words containing the letter. `score(word)` adds them up (repeated letters only once). The solver ranks suggestions by this score (§2.6).

---

## Self-check questions (Part 1)

1. Why does `Stack` push at the head of the singly linked list and not the tail?
2. What would go wrong if `SafeArray` used the compiler-generated copy constructor?
3. Draw the `ArrayQueue` array after: capacity 4, enqueue 1 2 3, dequeue, dequeue, enqueue 4 5 6. Where is `front_`?
4. Insert 1, 2, 3 into an AVL tree. Which rotation happens and what is the root?
5. Why can a B-tree keep all leaves on the same level, while a BST cannot?
6. A tree with 7 nodes and height 3 is complete. Is it necessarily perfect? Full?
7. Why does the hash table grow to a prime number of buckets, and when does it grow?
8. To get the top 10 of 10,000 scores, why use a min-heap (not a max-heap) of size 10?
9. Which structure holds the active games on the server, and what is its key?
10. Why is `LetterTracker::status_` indexed by `letter − 'A'` instead of searching a list?
