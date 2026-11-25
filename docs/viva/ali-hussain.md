# Wordle-DSA: Complete DSA Guide for the Viva

This guide covers **only the DSA**: every data structure and every algorithm in the project. For each one you get:

- **In simple words**: what it is
- **Picture**: how it looks in memory
- **Where in the game**: why we used it
- **Where in the code**: file and line, so you can open it
- **How it works**: step by step
- **Most important code**
- **Complexity** (Big-O)
- **Viva questions** with answers

Items marked ⭐ are the ones **you (Ali Hussain) wrote**. Expect the most questions on those, but the examiner can ask about anything.

> All numbers in this guide (comparisons, heights, sort counts) were measured by running our real code on our real `data/words.txt` (701 words). You can quote them.

---

## Contents

0. [Cheat sheet: every DSA in one table](#0-cheat-sheet-every-dsa-in-one-table)
1. [Basics: ADT, Big-O, Rule of Three, errors](#1-basics-adt-big-o-rule-of-three)
2. [Arrays](#2-arrays): ⭐SafeArray, ⭐2D Board, ⭐jagged keyboard, ⭐direct addressing, 5x26 FrequencyTable
3. [Linked lists](#3-linked-lists): singly, doubly, circular
4. [Stacks and queues](#4-stacks-and-queues): linked and array versions
5. [Trees](#5-trees): BST, tree properties, AVL, 2-3 tree (B-tree)
6. [⭐Hash table](#6-hash-table-)
7. [⭐Heap (priority queue)](#7-heap-priority-queue-)
8. [Searching](#8-searching): linear, binary, interpolation, on a linked list, the six-way comparison
9. [Sorting](#9-sorting): bubble, selection, insertion, comb, merge, quick, radix
10. [Recursion and simulation of recursion](#10-recursion-and-simulation-of-recursion)
11. [Backtracking (the solver)](#11-backtracking-the-solver)
12. [KMP string search](#12-kmp-string-search)
13. [Fisher-Yates shuffle](#13-fisher-yates-shuffle)
14. [⭐Two-pass colouring (the Wordle rule)](#14-two-pass-colouring-the-wordle-rule-)
15. [Why no graphs](#15-why-no-graphs)
16. [Master complexity table](#16-master-complexity-table)
17. [Viva question bank](#17-viva-question-bank)

---

## 0. Cheat sheet: every DSA in one table

| # | DSA | Used in the game for | File |
|---|-----|----------------------|------|
| 1 | ⭐ Safe dynamic array | Storing the dictionary and almost every list | `backend/include/ds/SafeArray.h` |
| 2 | ⭐ Dynamic 2D array | The game board (attempts × 5) | `backend/src/Board.cpp` |
| 3 | ⭐ Jagged array | On-screen keyboard rows (10, 9, 7 keys) | `backend/src/LetterTracker.cpp` |
| 4 | ⭐ Direct addressing (array of 26) | Colour of each letter A-Z | `backend/src/LetterTracker.cpp` |
| 5 | 2D array 5 × 26 | How often each letter appears at each position | `backend/src/FrequencyTable.cpp` |
| 6 | Singly linked list | Hash table chains, base of stack and queue, solver's word list | `backend/include/ds/SinglyLinkedList.h` |
| 7 | Doubly linked list | Game replay (forward and back) | `backend/include/ds/DoublyLinkedList.h` |
| 8 | Circular linked list | Picking the secret word with no repeats | `backend/include/ds/CircularLinkedList.h` |
| 9 | Stack (linked) | Guess history, AVL in-order walk | `backend/include/ds/Stack.h` |
| 10 | Stack (array) | Iterative quick sort, iterative backtracking | `backend/include/ds/ArrayStack.h` |
| 11 | Queue (linked) | Hint order, level-order tree walk | `backend/include/ds/Queue.h` |
| 12 | Queue (circular array) | Last 10 finished games | `backend/include/ds/ArrayQueue.h` |
| 13 | BST | Letters of the secret word | `backend/include/ds/BST.h` |
| 14 | AVL tree | Dictionary (lookup method 4) | `backend/include/ds/AVLTree.h` |
| 15 | 2-3 tree (B-tree order 3) | Dictionary (lookup method 5) | `backend/include/ds/BTree.h` |
| 16 | Binary tree properties | Full / complete / perfect checks | `backend/include/ds/BinaryTreeProperties.h` |
| 17 | ⭐ Hash table | Dictionary (lookup method 6, the real one), active games | `backend/include/ds/HashTable.h` |
| 18 | ⭐ Min-heap | Top-10 leaderboard | `backend/include/ds/MinHeap.h` |
| 19 | Linear / binary / interpolation search | Dictionary lookup methods 1-3 | `backend/include/algorithms/Search.h` |
| 20 | Search on a linked list | Solver's candidate list | `backend/include/algorithms/ListSearch.h` |
| 21 | Bubble, selection, insertion, comb sort | Sorting Lab | `backend/include/algorithms/Sort.h` |
| 22 | Merge sort | Leaderboard order | `Sort.h` |
| 23 | Quick sort | Ranking solver suggestions | `Sort.h` |
| 24 | Radix sort | Sorting the dictionary at startup | `Sort.h` |
| 25 | Recursion types | Tail, head, tree, indirect, nested | `backend/include/algorithms/Recursion.h` |
| 26 | Backtracking | Solver builds possible words letter by letter | `backend/src/Solver.cpp` |
| 27 | KMP | Search leaderboard by name | `backend/include/algorithms/StringSearch.h` |
| 28 | Fisher-Yates shuffle | Shuffle secret words and hint positions | `backend/include/algorithms/Shuffle.h` |
| 29 | ⭐ Two-pass evaluation | Green / yellow / gray with repeated letters | `backend/src/Wordle.cpp` |

---

## 1. Basics: ADT, Big-O, Rule of Three

### 1.1 ADT (Abstract Data Type)

An ADT says **what** a structure can do, not **how**. Example: a Stack ADT has `push`, `pop`, `top`. We wrote **two** stacks with the same ADT but different insides (`Stack` uses a linked list, `ArrayStack` uses an array). The user of the stack does not care which.

In our code, every structure is a `class` with **public** operations and **private** data (`head_`, `size_`, `data_`). That hiding is the ADT idea.

### 1.2 Big-O in simple words

Big-O says how the work grows when the input (n) grows.

| Big-O | Name | Example in our project | With n = 701 words |
|-------|------|------------------------|--------------------|
| O(1) | constant | hash table lookup, array index | ~1 step |
| O(log n) | logarithmic | binary search, AVL search | ~10 steps |
| O(n) | linear | linear search | up to 701 steps |
| O(n log n) | | merge sort, quick sort | ~6,600 steps |
| O(n²) | quadratic | bubble, selection, insertion sort | ~245,000 steps |

### 1.3 Rule of Three ⭐ (you will be asked this)

If a class uses `new` (owns heap memory), it must write 3 functions:

1. **Destructor** `~X()`: frees the memory with `delete`.
2. **Copy constructor** `X(const X& other)`: makes a **deep copy** (new memory, copy the values).
3. **Copy assignment** `X& operator=(const X& other)`: frees old memory, deep-copies, and checks `if (this == &other)` for self-assignment.

**Why?** Without them, C++ copies only the **pointer** (shallow copy). Two objects then point to the same memory. When both are destroyed, the memory is deleted twice → crash.

```
Shallow copy (BAD)          Deep copy (GOOD, what we do)
 a.data_ ─┐                  a.data_ ──► [1 2 3]
          ├─► [1 2 3]        b.data_ ──► [1 2 3]  (its own copy)
 b.data_ ─┘
```

**Who has it:** `SafeArray`, `HashTable`, `Board`, `LetterTracker`, all 3 linked lists, `ArrayStack`, `ArrayQueue`, `BST`, `AVLTree`, `BTree`, `FrequencyTable`.

**Who doesn't need it ("Rule of Zero"):** `Stack`, `Queue` and `MinHeap`. Their only member is a list or a `SafeArray`, which already copies itself correctly, so the default functions are fine.

**Where it really matters:** `Server.cpp:155` copies a whole `GameSession` (with its `Board`, lists, BST) into the hash table. If any of those did a shallow copy, the game would crash when the local copy is destroyed.

### 1.4 How errors work in our code

When something goes wrong (bad index, empty stack, bad word), the code **throws a plain string**:

```cpp
if (index >= size_)
    throw "SafeArray: index out of range";
```

Whoever called the function can **catch** it and read the message:

```cpp
try
{
    dictionary_.loadFromFile(wordsPath);
}
catch (const char* message)        // backend/src/Server.cpp
{
    cerr << "failed to load dictionary: " << message << "
";
}
```

The tests check errors with `CHECK_THROWS(s.pop());`, which passes only if `pop()` on an empty stack throws.

**Why throw instead of just returning something?** A function like `T& operator[]` must return an element; there is no "nothing" value to return for a bad index. Throwing stops the program going on with garbage.

### 1.5 `using namespace std;`

Every file has `using namespace std;` after its `#include` lines, so we write `string`, `cout` and `size_t` instead of `std::string`, `std::cout` and `std::size_t`. In `Server.cpp` the HTTP library is included **before** our headers, because it pulls in Windows headers that clash with some `std` names.

---

## 2. Arrays

### 2.1 ⭐ SafeArray: bounds-checked dynamic array

**In simple words:** our own version of `vector`. It grows when full, and checks every index so a wrong index throws an error instead of corrupting memory.

**Picture:**

```
data_ ──► [ CRANE | SLATE | PLUMB | ____ ]
            0       1       2       3
size_ = 3  (how many are used)
capacity_ = 4  (how many fit)
```

**Where in the game:** the dictionary's sorted word list (`Dictionary.h:165`), the heap's storage, the word picker, leaderboard entries, solver results, sorting lab arrays.

**Where in the code:** `backend/include/ds/SafeArray.h`

| Part | Line |
|------|------|
| Rule of Three | `:49` destructor, `:54` copy constructor, `:67` copy assignment |
| `pushBack` (doubling) | `:132` |
| `reserve` (allocate bigger, copy, free old) | `:189` |
| `checkIndex` (the safe part) | `:244` |

**How `pushBack` works:**

1. If `size_ == capacity_` (full), make a new array **twice as big** (0 → 4 → 8 → 16 ...), copy everything over, free the old one.
2. Put the value at `data_[size_]`, then `size_++`.

**Most important code:**

```cpp
void pushBack(const T& value)
{
    if (size_ == capacity_)
        reserve(capacity_ == 0 ? kInitialCapacity : capacity_ * 2);
    data_[size_] = value;
    ++size_;
}

void checkIndex(size_t index) const
{
    if (index >= size_)                       // checks size, not capacity
        throw "SafeArray: index out of range";
}
```

**Why double and not +1? (Amortised O(1))** To push n items with doubling, the total copying is 4 + 8 + 16 + ... + n < 2n. So on average each push costs O(1). With +1 each time, copying is 1 + 2 + 3 + ... + n = O(n²).

**Complexity:** index O(1), pushBack O(1) amortised, insertAt / removeAt O(n) (shifting).

### 2.2 ⭐ Dynamic 2D array: the Board

**In simple words:** the Wordle grid. Rows = attempts (8 Easy, 6 Normal, 5 Hard), columns = 5 letters. The number of rows is only known when the game starts, so it is built with `new` at run time.

**Picture (array of row pointers):**

```
letters_ ──► [ptr0] ──► [ C | R | A | N | E ]
             [ptr1] ──► [ S | L | A | T | E ]
             [ptr2] ──► [   |   |   |   |   ]
             ...
```

There are two such grids: `letters_` (the char) and `states_` (green / yellow / gray).

**Where in the code:** `backend/src/Board.cpp`: `allocate` `:87`, `release` `:103`, `addRow` `:45`, bounds check `:133`. Members in `backend/include/Board.h:48-49`.

**Most important code:**

```cpp
letters_ = new char*[rows_];              // step 1: array of pointers
for (int row = 0; row < rows_; ++row)
    letters_[row] = new char[cols_];      // step 2: one array per row
```

Freeing is the reverse: delete each row, then delete the pointer array. If you only delete `letters_`, every row leaks.

**Complexity:** access O(1); allocate / copy / free O(rows × cols).

### 2.3 ⭐ Jagged array: the keyboard

**In simple words:** a 2D array whose rows have **different lengths**.

```
keyRows_ ──► [ptr0] ──► Q W E R T Y U I O P   (10)
             [ptr1] ──► A S D F G H J K L     (9)
             [ptr2] ──► Z X C V B N M         (7)
rowLengths_ = [10, 9, 7]
```

Because rows differ, we **must store each row's length** in `rowLengths_`. A normal 3 × 10 array would waste 4 cells.

**Where:** `backend/src/LetterTracker.cpp:103` (`buildKeyboard`), `:8` (the three strings).

### 2.4 ⭐ Direct addressing: array of 26 letters

**In simple words:** the colour of each letter is stored at `status_[letter - 'A']`. `'C' - 'A' = 2`, so C is at index 2. No searching, always O(1). This is the simplest "perfect hash".

**Where:** `backend/src/LetterTracker.cpp:43` (`update`), `:73` (`indexOf`).

**Key rule: only upgrade.** The states are numbered `Unused 0 < Absent 1 < Present 2 < Correct 3` (`backend/include/LetterState.h:6`). A key only changes if the new colour is **better**:

```cpp
if ((int)(feedback[i]) > (int)(status_[index]))
    status_[index] = feedback[i];
```

Example: secret APPLE, guess PUPPY → the P's are yellow, green, gray. The keyboard key P must show **green**, not the last value (gray).

The same `letter - 'A'` trick is used in `Wordle.cpp` (`remaining[26]`), `Constraints.cpp` (`minCount_[26]`, `maxCount_[26]`) and `FrequencyTable.cpp`.

### 2.5 FrequencyTable: 5 × 26 2D array

**In simple words:** `counts_[position][letter]` = how many possible words have that letter at that position. Used to score suggestions: a word made of common letters in common places is a better guess.

**Where:** `backend/src/FrequencyTable.cpp`: allocate `:31`, `add` `:78`, `score` `:134`.

**Real result:** after guessing CRANE against secret APPLE, the top suggestions are TABLE (84), FABLE (83), FALSE (82).

---

## 3. Linked lists

### 3.1 Singly linked list

**In simple words:** a chain of nodes. Each node holds data and a pointer to the **next** node. We keep both `head_` and `tail_` pointers.

```
head_ ──► [CRANE|•]──► [SLATE|•]──► [PLUMB|null]
                                        ▲
tail_ ──────────────────────────────────┘
```

**Where in the game:**
- Each **hash table bucket** is a singly linked list (collision chain).
- The base of `Stack` and `Queue`.
- The **solver's candidate list** (words still possible).

**Where in the code:** `backend/include/ds/SinglyLinkedList.h`: `pushFront` `:121`, `pushBack` `:129`, `popFront` `:169`, `popBack` `:180`, `removeAllIf` `:249`, `findIf` `:340`, `unlinkAfter` `:388`.

**Key ideas:**
- `pushFront`: new node points to old head, head = new node. O(1).
- `pushBack`: O(1) **because we keep `tail_`**. Without a tail pointer we would walk the whole list.
- `popBack` is O(n): to remove the tail we need the node **before** it, and we can only move forward.
- `removeAllIf` (`:249`) removes every matching node in **one pass**. The solver uses it to delete all impossible words after each guess. In an array, each removal would shift elements; in a list we just relink pointers.

```cpp
// unlinkAfter: remove the node after `before`
Node* doomed = before->next;
before->next = doomed->next;      // skip over it
if (doomed == tail_) tail_ = before;
delete doomed;
```

### 3.2 Doubly linked list

**In simple words:** each node has **two** pointers: `prev` and `next`. You can walk both ways.

```
null ◄──[CRANE]◄──►[SLATE]◄──►[APPLE]──► null
          head                   tail
```

**Where in the game:** **game replay**. After a game, the player steps forward and backward through the guesses (`GameSession.cpp:166`: `replay_.pushBack(fb)`).

**Where in the code:** `backend/include/ds/DoublyLinkedList.h`: `pushBack` `:143`, `nodeAt` `:296`, `unlink` `:313`.

**Key ideas:**
- `popBack` is O(1) here (unlike singly) because the tail knows its `prev`.
- `nodeAt(i)` walks from **whichever end is closer**, so at most n/2 steps.

### 3.3 Circular linked list

**In simple words:** the last node points back to the first. There is no end. A **cursor** marks the current item.

```
   ┌──► BRAVE ──► CRANE ──► SLATE ──► PLUMB ──┐
   └──────────────────────────────────────────┘
               ▲ cursor
```

**Where in the game:** **picking the secret word**. All 701 words are shuffled and put in the circle. Each new game moves the cursor one step. So no word repeats until all 701 have been used. Then it reshuffles.

**Where in the code:** `backend/include/ds/CircularLinkedList.h`: `advance` `:237`, `cursorAtStart` `:251`. Used by `backend/src/WordPicker.cpp:20` (`nextWord`) and `:39` (`startNewCycle`).

**Key ideas:**
- We store **only `tail_`**. The head is always `tail_->next`. So both `pushFront` and `pushBack` are O(1).
- `advance()` is just `cursor_ = cursor_->next;` O(1).
- After a reshuffle, if the first new word equals the last served word, it is swapped to the end so the same word never comes twice in a row (`WordPicker.cpp:51`).

---

## 4. Stacks and queues

### 4.1 Stack: LIFO (Last In, First Out)

Like a pile of plates: you add and take from the top.

**Version 1, linked (`backend/include/ds/Stack.h`):** built on the singly linked list. **Top = head**, so `push` = `pushFront` and `pop` = `popFront`, both O(1). (If top were the tail, pop would be O(n), because popBack in a singly list is O(n).)

**Version 2, array (`backend/include/ds/ArrayStack.h`):** top = `data_[size_ - 1]`. Doubles when full (`grow` `:115`).

**Where in the game:**
- **Guess history**: `GameSession.cpp:165` `history_.push(guess)`. Most recent guess is on top, so the history panel shows newest first.
- **AVL in-order walk without recursion** (`AVLTree.h:160`).
- **Iterative quick sort** and **iterative backtracking** use `ArrayStack`.

**Linked vs array stack (a common question):**

| | `Stack` (linked) | `ArrayStack` (array) |
|-|---|---|
| push | O(1), one `new` per item | O(1) amortised, sometimes grows |
| pop | O(1), one `delete` | O(1), nothing freed |
| memory | extra pointer per item | some unused capacity |
| speed in practice | nodes scattered in memory | items next to each other (cache-friendly) |

### 4.2 Queue: FIFO (First In, First Out)

Like a line at a shop: join at the back, leave from the front.

**Version 1, linked (`backend/include/ds/Queue.h`):** `enqueue` = `pushBack` (O(1) thanks to `tail_`), `dequeue` = `popFront` (O(1)).

**Version 2, circular array (`backend/include/ds/ArrayQueue.h`):** a plain array queue would shift all items left on every dequeue (O(n)). A **circular array** avoids that: indices wrap around with `% capacity`.

```
capacity 6, size 4, front_ = 4
index:  0    1    2    3    4    5
      [ C ][ D ][   ][   ][ A ][ B ]
                            ^front
removal order: A, B, C, D
next free slot = (front_ + size_) % capacity_ = (4 + 4) % 6 = 2
```

```cpp
// ArrayQueue.h:84
data_[(front_ + size_) % capacity_] = value;   // enqueue
// ArrayQueue.h:106
front_ = (front_ + 1) % capacity_;             // dequeue
```

**Bounded mode (ring buffer):** when full, `enqueue` drops the **oldest** item instead of growing. Used for the server's **last 10 games** (`Server.h:75`).

**Where in the game:**
- **Hint order**: at game start, positions 0-4 are **shuffled** into a queue (`GameSession.cpp:42`). Each hint **dequeues** the next position, skipping positions the player already solved (`GameSession.cpp:59`).
- **Level-order tree walk** for the "is complete?" check (`BinaryTreeProperties.h:84`).
- **Recent games** (circular `ArrayQueue`).

---

## 5. Trees

### 5.1 BST (Binary Search Tree)

**In simple words:** each node has at most 2 children. **Smaller values go left, bigger go right.** So search can skip half the tree at each step.

**Where in the game:** the **letters of the secret word** (`BST<char>` in `Wordle.h`). Answers "is this letter in the word?".

**Where in the code:** `backend/include/ds/BST.h`: recursive `insert` `:138`, `contains` `:155`, counting version `:166`, `height` `:182`, `destroy` `:192`.

**How insert works (recursive):**

```cpp
Node* insert(Node* node, const T& value)
{
    if (node == nullptr) return new Node(value);      // empty spot: put it here
    if (value < node->data) node->left  = insert(node->left, value);
    else if (node->data < value) node->right = insert(node->right, value);
    // equal: duplicate, ignore
    return node;
}
```

**Problem with BST:** if you insert in sorted order (A, C, E, N, R), it becomes a straight line (height 5, like a linked list). **Our fix ⭐** (`Wordle.cpp:19`): sort the 5 letters, then insert the **middle first** (order of indices 2, 1, 4, 0, 3):

```
CRANE → sorted A C E N R → insert E, C, R, A, N

          E
        /   \
       C     R
      /     /
     A     N          height 3, not 5
```

**Complexity:** O(h) where h = height. Balanced: O(log n). Worst (sorted input): O(n).

### 5.2 Binary tree properties

**Where:** `backend/include/ds/BinaryTreeProperties.h`. Used on the BST and AVL tree, shown by `/api/dsa/trees`.

| Property | Meaning | How we check it |
|----------|---------|-----------------|
| **Full** | every node has 0 or 2 children | recursion: no node with exactly one child (`:71`) |
| **Complete** | every level full except maybe the last, and the last is filled from the left | **level-order walk with a Queue**: once a missing child is seen, no later node may have a child (`:84`) |
| **Perfect** | full and all leaves on the same level; then n = 2^h − 1 | count nodes, compare with 2^h − 1 (`:116`) |

```
full, not complete     complete, not full      perfect
       A                      A                   A
      / \                    / \                 / \
     B   C                  B   C               B   C
        / \                /                   / \ / \
       D   E              D                   D  E F  G
```

Node count, leaf count and height are all **tree recursion**: `1 + count(left) + count(right)`.

### 5.3 AVL tree (self-balancing BST)

**In simple words:** a BST that **fixes itself** after each insert so it never becomes a line. Rule: for every node, the heights of the left and right subtrees differ by **at most 1**. That difference is the **balance factor** = height(left) − height(right).

**Where in the game:** the **dictionary** (lookup method 4). All 701 words are inserted.

**Where in the code:** `backend/include/ds/AVLTree.h`: `rotateRight` `:236`, `rotateLeft` `:255`, `balance` `:268`, `insert` `:300`, iterative in-order `toSortedArray` `:160`.

**The 4 rotation cases (must know):**

| Case | Shape | Fix |
|------|-------|-----|
| **LL** | too heavy on the left, in the left child's left | one **right** rotation |
| **RR** | too heavy on the right, in the right child's right | one **left** rotation |
| **LR** | heavy left, but in the left child's **right** | rotate left child **left**, then node **right** |
| **RL** | heavy right, but in the right child's **left** | rotate right child **right**, then node **left** |

**Right rotation (LL case):**

```
       node             left
       /  \             /   \
     left   C   =>     A    node
     /  \                   /  \
    A    B                 B    C
```

```cpp
static Node* balance(Node* node)
{
    updateHeight(node);
    int bf = balanceFactor(node);
    if (bf > 1) {                                   // left-heavy
        if (balanceFactor(node->left) < 0)          // LR: fix child first
            node->left = rotateLeft(node->left);
        return rotateRight(node);                   // LL
    }
    if (bf < -1) {                                  // right-heavy
        if (balanceFactor(node->right) > 0)         // RL: fix child first
            node->right = rotateRight(node->right);
        return rotateLeft(node);                    // RR
    }
    return node;
}
```

Each node **stores its height**, so `height()` is O(1) and the balance check is fast.

**Real numbers:** 701 words → AVL height **10**, **351 leaves**. The best possible height for 701 nodes is ⌈log₂(702)⌉ = 10, so our AVL is as short as possible.

**Iterative in-order traversal (simulation of recursion)** (`AVLTree.h:160`): instead of recursion, use our own `Stack`:

1. Go left as far as possible, pushing each node.
2. Pop a node, visit it.
3. Go to its right child, repeat.

This gives words in sorted order. Tests use it to prove the tree stays sorted after rotations.

**Complexity:** insert / search O(log n) **guaranteed** (height ≤ ~1.44 log₂ n).

### 5.4 2-3 tree (B-tree of order 3)

**In simple words:** a search tree where a node can hold **1 or 2 keys** and have **2 or 3 children**. **All leaves are always on the same level.** The tree grows **upward** (at the root), not downward.

```
                [ GHOST | PLUMB ]
               /        |        \
       [ APPLE ]   [ LEMON ]   [ STONE | ZEBRA ]
```

**Where in the game:** the dictionary again (lookup method 5). Shows how fatter nodes give a shorter tree.

**Where in the code:** `backend/include/ds/BTree.h`: `insert` `:100`, `search` `:139`, `insertInto` `:213`, `splitChild` `:240`.

**How insert works (bottom-up split):**

1. Walk down to the correct **leaf** and put the key there (in sorted order).
2. If the node now has **3 keys** (one too many), **split** it: the **middle key moves up** to the parent; left and right keys become two nodes.
3. The parent may now overflow → split it too.
4. If the **root** splits, make a **new root** above it. Height grows by 1. That is the only way height grows, so all leaves stay on one level.

**Search:** in each node, compare with its keys left to right; if equal → found; if smaller → go to that child; else try the next key.

**Real numbers:** 701 words → height **9**, **695 nodes**. Why not much shorter than AVL (10)? We insert the words in **sorted order**, and bottom-up splits then leave most nodes with only 1 key. With random order or a bigger order (e.g. `BTree<T, 5>`) it gets much shorter.

**Complexity:** O(log n) search and insert.

---

## 6. Hash table ⭐

**In simple words:** an array of "buckets". A **hash function** turns the key (a word) into a number, and `number % bucketCount` says which bucket to use. Lookup goes straight to that bucket, so it is O(1) on average.

**Picture (separate chaining):**

```
key "CRANE" → hash → huge number → % 1597 → bucket 2

buckets_:
 [0]    → null
 [1]    → {SLATE} → null
 [2]    → {CRANE} → {PLUMB} → null     ← collision: same bucket, one chain
 ...
 [1596] → null
```

**Where in the game:**
1. **Dictionary**: "is this a real word?" on every guess. This is the method the game **actually uses**; the other 5 just run beside it for comparison.
2. **Active games**: the server finds a game by its `gameId` (`Server.h:62`, `HashTable<string, GameSession>`).
3. **Removing duplicates** while loading words: `insert` returns `false` for a duplicate (`Dictionary.cpp:116`).

**Where in the code:** `backend/include/ds/HashTable.h`

| Part | Line |
|------|------|
| String hash `hashKey(string)` (polynomial rolling, base 31) | `:49` |
| Number hash `hashKey(long long)` (the number itself) | `:57` |
| Default 11 buckets, max load factor 0.75 | `:77-78` |
| `insert` | `:118` |
| `contains` with comparison counter | `:144` |
| `bucketIndex` = hash % buckets | `:255` |
| `rehash` | `:266` |
| `isPrime`, `nextPrime` | `:321`, `:336` |

**The hash function (Horner's rule):**

```cpp
inline uint64_t hashKey(const string& key)
{
    uint64_t hash = 0;
    for (char c : key)
        hash = hash * 31 + (unsigned char)c;   // wraps around at 2^64
    return hash;
}
```

This equals `s[0]·31⁴ + s[1]·31³ + s[2]·31² + s[3]·31 + s[4]`. **Position matters**, so anagrams like SLATE / STEAL / LEAST get **different** hashes. (Adding the letters up would put all anagrams in one bucket.)

**Why 31?** An odd prime: multiplying by an even number would lose a bit every step. It is fast and is the classic choice (Java's `String.hashCode` uses it).

**Insert step by step:**

1. Find the bucket for the key.
2. Walk the chain; if the key exists, update its value and return `false`.
3. Else put the new entry at the **front** of the chain (O(1)), `size++`.
4. If **load factor** (size ÷ buckets) > **0.75**, **rehash**.

**Rehashing:** make a new bucket array of size `nextPrime(2 × old)`, re-insert **every** entry (its bucket changes because `% bucketCount` changed), free the old array.

**Why prime bucket counts?** With 32 buckets, `hash % 32` only looks at the last 5 bits of the hash. A prime uses all the bits, so keys spread more evenly.

**Real numbers with our 701 words:**

| | |
|-|-|
| Start | 11 buckets |
| Rehashes | **7** (11 → 23 → 47 → 97 → 197 → 397 → 797 → **1597**) |
| Load factor | 701 / 1597 = **0.44** |
| Buckets used | 569 |
| Longest chain | **3** |
| Comparisons to find CRANE | **1** |

**Collision handling, ours vs the other way:**

| Separate chaining (ours) | Open addressing (not ours) |
|---|---|
| each bucket is a linked list | everything in one array; on collision, try the next slot (linear / quadratic probing, double hashing) |
| never "full" | full when every slot is used |
| delete = unlink a node | delete needs "tombstone" markers |

**Complexity:** insert / search / delete **O(1) average**, **O(n) worst** (every key in one bucket). Rehash O(n).

---

## 7. Heap (priority queue) ⭐

**In simple words:** a **complete binary tree** stored in an **array**, where every parent is **≤ its children** (min-heap). So the **smallest** item is always at index 0.

**Picture:**

```
index:   0   1   2   3   4   5
value:  10  20  15  40  30  50

            10              ← minimum at index 0
          /    \
        20      15
       /  \    /
      40  30  50

parent(i) = (i - 1) / 2     left(i) = 2i + 1     right(i) = 2i + 2
```

No pointers are needed: the positions in the array **are** the tree.

**Where in the game:** **top-10 leaderboard**. Every saved score is streamed through a min-heap of size 10.

**Where in the code:** `backend/include/ds/MinHeap.h`: heapify constructor `:53`, `push` `:62`, `pop` `:69`, `replaceTop` `:81`, **`pushBounded`** `:94`, `siftUp` `:162`, `siftDown` `:174`. Used at `backend/src/Leaderboard.cpp:113-115`.

**Operations:**
- **push**: add at the end, then **sift up** (swap with the parent while smaller). O(log n).
- **pop** (remove min): move the last item to the root, then **sift down** (swap with the **smaller** child until the order is right). O(log n).
- **top**: `data_[0]`. O(1).

**Why a MIN-heap for the TOP 10? (they will ask)** We need quick access to the **weakest of the current top 10**, because that is the score a new one must beat and the one that gets thrown out. The min-heap keeps it at the root.

```cpp
bool pushBounded(const T& value, size_t capacity)   // capacity = 10
{
    if (data_.size() < capacity) { push(value); return true; }     // not full: just add
    if (compare_(data_[0], value)) { replaceTop(value); return true; } // beats the weakest
    return false;                                                   // too small, ignore
}
```

Total cost: **O(n log 10)** time and only **10** items in memory, instead of sorting all n scores (O(n log n)).

**The heap array is not sorted** (only parent ≤ child). So after the heap, the 10 survivors are put in order with **merge sort** (`Leaderboard.cpp:125`).

**Heapify in O(n)** (`:53`): to build a heap from an array, sift down every non-leaf, from the last one back to the root. Half the nodes are leaves (no work) and most others are near the bottom, so the total is O(n), not O(n log n).

**Heap sort:** `drainSorted` (`:130`) pops the minimum n times → sorted output, O(n log n).

**Min-heap → max-heap:** the heap takes a comparator template parameter. Pass "greater than" and the same code becomes a max-heap.

---

## 8. Searching

All 6 dictionary lookup methods run on **every guess**, and the DSA Insights panel shows the comparisons for each (`Dictionary.cpp:218`, `lookupAll`). Each is also timed by repeating it **200 times** and taking the average, because one lookup is faster than the clock can measure.

**Real counts for the word CRANE (701 words):**

| Method | Comparisons | Big-O | Needs |
|--------|-------------|-------|-------|
| Linear search | **158** | O(n) | nothing |
| Binary search | **7** | O(log n) | sorted array |
| Interpolation search | **11** | O(log log n) avg, O(n) worst | sorted + evenly spread keys |
| AVL tree | **13** | O(log n) | the tree |
| 2-3 tree | **15** | O(log n) | the tree |
| ⭐ Hash table | **1** | O(1) avg | the table |

### 8.1 Linear search

Check every item from the start. O(n). `backend/include/algorithms/Search.h` (`linearSearch`). The baseline.

### 8.2 Binary search (recursive)

On a **sorted** array: look at the middle; if the key is smaller go left, else go right. Half is thrown away each step. For 701 words, at most ⌈log₂ 702⌉ = **10** comparisons.

```cpp
int binarySearch(const Arr& a, int low, int high, const T& key, int* comparisons)
{
    if (low > high) return -1;                  // not found
    int mid = low + (high - low) / 2;           // avoids int overflow of (low+high)
    if (a[mid] == key) return mid;
    if (key < a[mid]) return binarySearch(a, low, mid - 1, key, comparisons);
    return binarySearch(a, mid + 1, high, key, comparisons);   // tail recursion
}
```

Why `low + (high - low) / 2` and not `(low + high) / 2`? With huge indexes `low + high` can overflow an `int`.

### 8.3 Interpolation search

Instead of the middle, **guess** where the key should be, like opening a phone book near "S" for "Smith".

```
pos = low + (key - a[low]) × (high - low) / (a[high] - a[low])
```

Words are not numbers, so each word is turned into a **base-26 number** (`wordToKey`, `Search.h`): A=0 ... Z=25, `key = key × 26 + letter`. Now sorted words have numeric keys.

**Why did it need more comparisons (11) than binary (7) for CRANE?** It is only fast when keys are **evenly spread**. English words are not: many start with S, few with X or Z. So its guesses can be off.

### 8.4 Searching a linked list (`backend/include/algorithms/ListSearch.h`)

Used on the solver's candidate list.

- **Linear search**: same as an array. O(n).
- **Binary search**: a list has no index, so the **middle is found with slow / fast pointers**: `fast` moves 2 steps for every 1 step of `slow`; when fast reaches the end, slow is in the middle.
- Result: **comparisons are O(log n)**, but **pointer moves are O(n)** (n/2 + n/4 + ...). This shows why binary search is only worth it with O(1) random access (an array).
- Interpolation search is **impossible** on a list: it needs to jump straight to a computed position.

### 8.5 Why tree search counts more than binary search

AVL (13) and 2-3 tree (15) count more than binary search (7) even though all are O(log n): each tree node can cost **2** comparisons (`==` then `<`), and a 2-3 node can hold 2 keys (up to 4 comparisons per node).

---

## 9. Sorting

All sorts are in `backend/include/algorithms/Sort.h`. They count comparisons and moves for the **Sorting Lab** (`backend/src/SortingLab.cpp`), which sorts a shuffled copy of all 701 words with each algorithm.

**Real Sorting Lab results (701 shuffled words):**

| Sort | Comparisons | Moves | Big-O | Stable? |
|------|-------------|-------|-------|---------|
| Bubble | 245,305 | 119,495 | O(n²) | yes |
| Selection | 245,350 | **697** | O(n²) | no |
| Insertion | 120,194 | 119,495 | O(n²) | yes |
| Comb | 16,630 | 2,973 | ~O(n log n) in practice | no |
| Merge | **5,756** | 6,687 | O(n log n) | yes |
| Quick | 7,727 | 3,065 | O(n log n) avg, O(n²) worst | no |
| Radix | **0** | 3,505 | O(L × (n + 26)), L = 5 | yes |

(**Stable** = equal items keep their original order.)

### 9.1 Bubble sort (`Sort.h:44`)

Compare neighbours; swap if in the wrong order. After each pass the biggest item "bubbles" to the end. **Early exit**: if a pass makes no swap, the array is sorted, so sorted input costs only O(n).

### 9.2 Selection sort (`Sort.h:67`)

Find the smallest item in the unsorted part and swap it to the front. Always n(n−1)/2 comparisons, but **at most n − 1 swaps**: see 697 moves above. Best when moving items is expensive.

### 9.3 Insertion sort (`Sort.h:116`)

Take the next item and slide it left until it is in place (like sorting cards in your hand). O(n) on almost-sorted data. **Used in the game:** sorting the 5 letters of the secret word before building the BST (`Wordle.cpp`).

### 9.4 Comb sort (`Sort.h:89`)

Bubble sort with a **gap**: compare items far apart first. The gap starts at n and shrinks by ÷1.3 each pass. This moves small items stuck at the end ("turtles") forward fast. The last passes use gap 1 (plain bubble sort).

### 9.5 Merge sort (`Sort.h:135-180`) — divide and conquer

1. **Divide**: split the array into two halves.
2. **Conquer**: sort each half recursively.
3. **Combine**: **merge** the two sorted halves into one using a temp array.

```
[38 27 43 3]
   /      \
[38 27]  [43 3]
 / \      / \
38 27    43  3
 \ /      \ /
[27 38]  [3 43]
    \     /
 [3 27 38 43]
```

Always O(n log n) (log n levels, O(n) merging per level). Needs O(n) extra memory. **Stable.** **Used in the game:** ordering the top-10 leaderboard (`Leaderboard.cpp:125`), because stability keeps equal scores in a fixed order.

### 9.6 Quick sort (`Sort.h:202-294`)

1. Pick a **pivot**.
2. **Partition**: smaller items to the left, bigger to the right. The pivot lands in its final place.
3. Recursively sort the left and right parts.

**Median-of-three pivot** (`quickPartition` `:202`): the pivot is the middle value of `a[low]`, `a[mid]`, `a[high]`. Plain "last item as pivot" on **already sorted** input (our dictionary after radix sort!) gives the worst case O(n²). Median-of-three avoids that.

**Partition: Lomuto scheme** (`:214`): `boundary` marks the end of the "smaller than pivot" part; every smaller item is swapped there.

**Two versions:**
- `quickSort`: recursive (tree recursion: 2 calls).
- `quickSortIterative` (`:253`): **simulation of recursion** with our own `ArrayStack` of ranges. It pushes the **larger** part first so the smaller one is handled next, which keeps the stack only O(log n) deep.

**Used in the game:** ranking the solver's suggestions by score (`Solver.cpp:193`).

### 9.7 Radix sort, LSD (`Sort.h:309`)

**No comparisons at all.** Sort by the **last** letter, then the 4th, ... then the 1st, each time using a stable **counting sort** with 26 buckets.

```
Example with 3-letter words: BAD CAB ABC
by letter 3:  CAB(B) ABC(C) BAD(D)
by letter 2:  CAB(A) BAD(A) ABC(B)     (stable: CAB stays before BAD)
by letter 1:  ABC(A) BAD(B) CAB(C)     sorted!
```

Each pass: count each letter, turn counts into start positions (prefix sums), place every word in its spot.

**Why it fits perfectly:** every word has exactly **5 letters** from a **26-letter** alphabet. So it costs O(5 × (n + 26)) = **linear**. **Used in the game:** sorting the dictionary once at startup (`Dictionary.cpp`, `radixSortWords`).

---

## 10. Recursion and simulation of recursion

**Recursion** = a function calls itself on a smaller problem. It always needs a **base case** (when to stop), or it runs until the call stack overflows.

**Types (all in `backend/include/algorithms/Recursion.h`):**

| Type | Meaning | Example in Recursion.h | Also used in the project |
|------|---------|------------------------|--------------------------|
| **Tail** | the recursive call is the **last** thing done | `countLetterTail` (`:30`) | binary search |
| **Head** | the recursive call is **first**; work happens on the way back | `reverseHead` (`:40`) | BST / AVL `destroy` (children freed before the node) |
| **Tree** | **more than one** recursive call per call | `countOrderings`, `fibonacciTree` (`:53`, `:65`) | merge sort, quick sort, node counts |
| **Indirect** | A calls B, B calls A | `alternatesFromVowel` ↔ `alternatesFromConsonant` (`:83`, `:90`) | |
| **Nested** | a recursive call is the **argument** of a recursive call | `mcCarthy91` (`:105`): `f(f(n+11))` | |

**Tree recursion can repeat work:** naive Fibonacci of 20 makes **21,891 calls** (measured), because the same values are computed again and again.

**Simulation of recursion** = replace recursion with **your own stack** (the computer's call stack is just a stack too). Three places in our project:

1. **AVL in-order traversal** with `Stack` (`AVLTree.h:160`).
2. **Iterative quick sort** with `ArrayStack` (`Sort.h:253`).
3. **Iterative backtracking** with `ArrayStack` (`Solver.cpp:123`).

Why? Deep recursion on big input can overflow the call stack; our own stack lives on the heap and can grow.

---

## 11. Backtracking (the solver)

**In simple words:** build the answer **one choice at a time**. If a partial answer can't work, **undo the last choice** and try the next one. Skipping a dead branch early is called **pruning**.

**Where in the game:** the **Solver** panel lists every dictionary word that still fits all the colours so far.

**Where in the code:** `backend/src/Solver.cpp:82` (`extend`, recursive) and `:123` (`generateCandidatesIterative`, own stack). The rules come from `backend/src/Constraints.cpp`.

**What the feedback tells us (`Constraints`):**
- `fixed_[pos]`: a green → that letter must be at that position.
- `banned_[pos][letter]` (5 × 26 2D array): a yellow or gray → letter not allowed at this position.
- `minCount_[letter]` / `maxCount_[letter]`: e.g. a gray on a letter that is also yellow means the count is **exact**.

**The algorithm: choose → explore → un-choose:**

```cpp
void Solver::extend(string& prefix, ...)
{
    if (prefix.size() == 5) { out.pushBack(prefix); return; }   // complete word found
    for (char c = 'A'; c <= 'Z'; ++c)
    {
        if (!constraints_.allowsLetterAt(depth, c)) { pruned++; continue; }  // prune
        prefix.push_back(c);                                                  // choose
        if (constraints_.fitsCounts(prefix) && isDictionaryPrefix(prefix))
            extend(prefix, ...);                                              // explore
        else
            pruned++;                                                         // prune
        prefix.pop_back();                                                    // un-choose
    }
}
```

**A branch is pruned when:**
1. the colours rule out this letter here,
2. the letter counts can no longer work out,
3. **no dictionary word starts with this prefix** (checked by **binary search** on the sorted dictionary, `isDictionaryPrefix` `:48`).

**Without pruning:** 26⁵ = **11,881,376** words to try.
**Real numbers:** before any guess the search visits **33,150** letters (701 words found). After guessing CRANE when the secret is APPLE, it visits only **13,364** and finds the **16** words still possible.

The **candidate linked list** gives the same 16 words a different way: after each guess, `removeAllIf` unlinks every word that breaks the rules in one pass (removed **685** of 701 here).

---

## 12. KMP string search

**In simple words:** find a pattern (e.g. "ali") inside a text (e.g. "Ali Hussain") **without going backwards in the text**.

**Where in the game:** **search the leaderboard by part of a name** (`Leaderboard.cpp:105`, case-insensitive).

**Where in the code:** `backend/include/algorithms/StringSearch.h`: `buildLps` `:21`, `kmpSearch` `:51`.

**Brute force problem:** on a mismatch it moves the start by 1 and re-checks characters it already saw. Worst case O(n × m).

**KMP idea:** first build an **LPS table** (Longest Prefix that is also a Suffix) for the pattern. On a mismatch, LPS says how far into the pattern you can **jump back** while **never moving backward in the text**.

```
pattern:  A B A B C
lps:      0 0 1 2 0
```

`lps[3] = 2` means "ABAB" ends with "AB", which is also how it starts. So after matching ABAB and failing on the next character, we already know "AB" matches and continue from pattern index 2.

**Complexity:** O(n + m): O(m) to build LPS, O(n) to scan.

---

## 13. Fisher-Yates shuffle

**In simple words:** a fair shuffle. Go from the last item to the first; swap each item with a **random** item at or before it.

```cpp
for (int i = n - 1; i > 0; i--)
{
    int j = random number in [0, i];
    swap(a[i], a[j]);
}
```

**Where in the game:** shuffling the 701 secret words before they go into the circular list (`WordPicker.cpp:48`), and the 5 hint positions before they go into the hint queue (`GameSession.cpp:50`).

**Where in the code:** `backend/include/algorithms/Shuffle.h`.

**Why this way?** Every one of the n! orders is **equally likely**. O(n) time, O(1) extra memory. (Swapping with a random index from the **whole** array each time is a common mistake: it is biased.)

---

## 14. Two-pass colouring (the Wordle rule) ⭐

**Where:** `backend/src/Wordle.cpp:45` (`evaluate`).

**Problem:** repeated letters. A simple one-pass check gives wrong colours.

**Algorithm:**

1. Count each letter in the secret: `remaining[26]` (direct addressing).
2. **Pass 1, greens:** every exact match is green and uses up one count.
3. **Pass 2, yellows / grays:** for the rest, if the letter is in the secret (BST check) **and** `remaining[letter] > 0` → yellow, use one count. Otherwise gray.

```cpp
int remaining[26] = {0};
for (i...) ++remaining[secret[i] - 'A'];
for (i...) if (guess[i] == secret[i]) { states[i] = Correct; --remaining[...]; }   // pass 1
for (i...) {                                                                        // pass 2
    if (states[i] == Correct) continue;
    if (containsLetter(guess[i]) && remaining[guess[i] - 'A'] > 0)
        { states[i] = Present; --remaining[...]; }
    else states[i] = Absent;
}
```

**Example 1:** secret APPLE, guess PUPPY → 🟨⬜🟩⬜⬜. APPLE has two P's: the middle P is green, the first P is yellow, the last P has none left so it is gray.

**Example 2, why greens go first:** secret CRANE (one E), guess EERIE.

```
One pass left to right (WRONG): first E takes the only E → yellow, last E green
                                → looks like two E's
Two passes (OURS):              last E green first, the other E's gray
                                → ⬜⬜🟨⬜🟩  (correct)
```

**Honest note if asked "is the BST check needed?":** strictly no. If `remaining[letter] > 0` the letter is in the word anyway. The BST is kept because it is the classic structure for "is this letter in the word?" (from the reference project), and its comparison count is shown in the DSA panel. `remaining[]` is what makes repeated letters correct.

---

## 15. Why no graphs

Graphs (MST, shortest path, topological sort) are the only course topic not used. Wordle has no network, map or dependency structure, so adding a graph would be artificial. (If pushed: you *could* link words that differ by one letter into a graph, but the game never needs it.)

---

## 16. Master complexity table

| Structure / algorithm | Main operation | Average | Worst | Extra space |
|-----------------------|----------------|---------|-------|-------------|
| ⭐ SafeArray | index / pushBack | O(1) / O(1) amortised | O(n) on resize | O(n) |
| ⭐ 2D Board | cell access | O(1) | O(1) | O(rows × cols) |
| ⭐ Letter array of 26 | lookup | O(1) | O(1) | O(26) |
| Singly linked list | push front / back, pop front | O(1) | O(1) | O(n) |
| Singly linked list | pop back, search | O(n) | O(n) | |
| Doubly linked list | push / pop both ends | O(1) | O(1) | O(n) |
| Circular linked list | advance cursor | O(1) | O(1) | O(n) |
| Stack / Queue (both kinds) | push, pop / enqueue, dequeue | O(1) | O(n) on array grow | O(n) |
| BST | search / insert | O(log n) | O(n) | O(n) |
| AVL tree | search / insert | O(log n) | O(log n) | O(n) |
| 2-3 tree | search / insert | O(log n) | O(log n) | O(n) |
| ⭐ Hash table | search / insert / delete | O(1) | O(n) | O(n + m) |
| ⭐ Min-heap | push / pop | O(log n) | O(log n) | O(n) |
| ⭐ Min-heap | top / build | O(1) / O(n) | | |
| Linear search | | O(n) | O(n) | O(1) |
| Binary search | | O(log n) | O(log n) | O(log n) recursion |
| Interpolation search | | O(log log n) | O(n) | O(1) |
| Bubble / selection / insertion | | O(n²) | O(n²) | O(1) |
| Comb sort | | ~O(n log n) | O(n²) | O(1) |
| Merge sort | | O(n log n) | O(n log n) | O(n) |
| Quick sort | | O(n log n) | O(n²) | O(log n) |
| Radix sort (LSD) | | O(L(n + k)) | O(L(n + k)) | O(n + k) |
| KMP | | O(n + m) | O(n + m) | O(m) |
| Fisher-Yates | | O(n) | O(n) | O(1) |
| Backtracking solver | | much less than 26⁵ thanks to pruning | 26⁵ | O(5) depth |
| ⭐ Two-pass evaluate | | O(5) | O(5) | O(26) |

---

## 17. Viva question bank

### Basics

**Q: What is an ADT?**
A: A description of what a structure does (its operations), separate from how it is built. Our `Stack` and `ArrayStack` are one ADT with two implementations.

**Q: How does your code report errors?**
A: It throws a plain string, e.g. `throw "Stack: the stack is empty";`. The caller catches it with `catch (const char* message)`.

**Q: Why `(int)x` casts?**
A: Sizes are `size_t` (unsigned). Where we compare a size with an `int` we convert it with `(int)` so both sides have the same type and the compiler does not warn.

**Q: What is the Rule of Three, and why?**
A: A class that owns heap memory needs a destructor, copy constructor and copy assignment. Otherwise copies share the same memory and it gets deleted twice.

**Q: Why does `MinHeap` not write its own destructor?**
A: Its only member is a `SafeArray`, which already handles its own memory and copying. The default ones are correct (Rule of Zero).

**Q: What is a deep copy?**
A: Allocating new memory and copying the values, not just the pointer.

**Q: Why the `if (this == &other)` check in `operator=`?**
A: For `a = a`. Without it the object would free its own memory and then copy from the freed memory.

### Arrays

**Q: What makes SafeArray safe?**
A: Every `[]` checks the index against `size_` and throws an error (`throw "SafeArray: index out of range";`) if it's wrong.

**Q: Why double the capacity?**
A: It makes `pushBack` O(1) amortised. Growing by 1 each time would make n pushes cost O(n²).

**Q: Difference between a 2D array and a jagged array?**
A: In a 2D array all rows have the same length (the board, 5 columns). In a jagged array rows differ (keyboard 10, 9, 7), so each row's length must be stored.

**Q: Why is the board allocated dynamically?**
A: The number of rows (8, 6 or 5) depends on difficulty, known only at run time.

**Q: What is direct addressing?**
A: Using the key itself as the index: `status_[letter - 'A']`. O(1), no collisions, works because there are only 26 letters.

### Linked lists

**Q: Why keep a tail pointer?**
A: So `pushBack` is O(1). The queue relies on it.

**Q: Why is `popBack` O(n) in a singly list but O(1) in a doubly list?**
A: To remove the tail you need the node before it. In a singly list you must walk from the head; in a doubly list the tail has a `prev` pointer.

**Q: Why a circular list for secret words?**
A: The cursor goes round the shuffled words; each game takes the next one, so no word repeats until all have been used, and wrapping around needs no special case.

**Q: Why does the circular list store only the tail?**
A: The head is `tail_->next`, so with one pointer both ends are reachable in O(1).

**Q: Why a doubly list for replay?**
A: The player steps forward and backward through the guesses, so we need both `next` and `prev`.

### Stacks and queues

**Q: Where is the stack's top in the linked stack, and why?**
A: At the head, so push and pop are O(1). At the tail, pop would be O(n).

**Q: Why a circular array for the array queue?**
A: A plain array queue shifts all items on every dequeue (O(n)). With wrap-around (`% capacity`), dequeue is O(1).

**Q: How do hints use a queue?**
A: The 5 positions are shuffled into a queue at game start. Each hint dequeues positions until it finds one the player hasn't solved yet.

**Q: What is a ring buffer?**
A: A fixed-size circular queue that overwrites the oldest item when full. We use it for the last 10 games.

### Trees

**Q: What is the BST property?**
A: Left subtree smaller than the node, right subtree bigger.

**Q: What is the worst case of a BST and how do you avoid it for the secret letters?**
A: Sorted input makes it a line (O(n)). We sort the 5 letters and insert the middle first, giving height 3.

**Q: What is the AVL balance factor?**
A: height(left) − height(right). It must stay −1, 0 or +1.

**Q: Explain the LR case.**
A: The node is left-heavy, but the extra height is in the left child's **right** subtree. Rotate the left child left (turning it into LL), then rotate the node right.

**Q: What is the AVL height for our dictionary?**
A: 10 for 701 words, the minimum possible (⌈log₂ 702⌉ = 10).

**Q: Full vs complete vs perfect?**
A: Full: every node has 0 or 2 children. Complete: all levels full except the last, which is filled from the left. Perfect: full and all leaves at the same depth (n = 2^h − 1).

**Q: How do you check "complete"?**
A: Level-order walk with a queue. After the first missing child, no later node may have a child.

**Q: How does a 2-3 tree stay balanced?**
A: Overfull nodes split and push the middle key up. The tree only grows at the root, so all leaves stay on the same level.

**Q: Why is our 2-3 tree (height 9) only a little shorter than the AVL tree (10)?**
A: Words are inserted in sorted order, so bottom-up splits leave most nodes with one key. Random order or a higher order would make it much shorter.

### Hash table ⭐

**Q: How does your hash function work?**
A: Polynomial rolling hash with Horner's rule: `h = h × 31 + c` for each character. Position matters, so anagrams get different hashes.

**Q: How are collisions handled?**
A: Separate chaining: each bucket is a singly linked list; colliding keys share a chain.

**Q: What is the load factor and when do you rehash?**
A: Entries ÷ buckets. When it goes above 0.75, we grow to the next prime ≥ 2 × the old size and re-insert everything.

**Q: Why re-insert everything on rehash?**
A: The bucket is `hash % bucketCount`. When bucketCount changes, every key's bucket changes.

**Q: Why prime bucket counts?**
A: Keys spread more evenly; a power of two would use only the low bits of the hash.

**Q: Real numbers?**
A: 701 words, 7 rehashes, 1597 buckets, load factor 0.44, longest chain 3, CRANE found in 1 comparison.

**Q: Worst case and why it's rare?**
A: O(n) if all keys land in one chain. A good hash plus rehashing keeps chains very short (longest here is 3).

**Q: Chaining vs open addressing?**
A: Chaining uses linked lists per bucket and never fills up. Open addressing probes other slots in one array and needs tombstones for delete.

### Heap ⭐

**Q: Why a min-heap for the top 10?**
A: The root is the weakest of the current top 10, the one a new score must beat. Checking is O(1), replacing it is O(log 10).

**Q: Formulas for parent and children?**
A: parent = (i − 1) / 2, left = 2i + 1, right = 2i + 2.

**Q: Is the heap array sorted?**
A: No, only parent ≤ child. That's why merge sort orders the final 10.

**Q: Why is building a heap O(n)?**
A: Half the nodes are leaves and need no work; most others are near the bottom and sift only a little. The total is O(n).

**Q: Leaderboard complexity?**
A: O(n log K) with K = 10, using only O(K) memory.

### Searching

**Q: Why does binary search need sorted data?**
A: It decides which half to throw away by comparing with the middle; that only works if the order is known.

**Q: When is interpolation search good, and why was it worse than binary for CRANE?**
A: When keys are evenly spread. English words aren't (many S, few X), so its position guesses can miss.

**Q: How do you turn a word into a number for interpolation search?**
A: Base 26: A = 0 ... Z = 25, `key = key × 26 + letter`.

**Q: Can you binary search a linked list?**
A: Yes, using slow / fast pointers to find each middle. Comparisons are O(log n), but pointer moves are O(n), so it gains nothing over linear search.

### Sorting

**Q: Which sort is used where in the game?**
A: Radix sort for the dictionary at startup, merge sort for the leaderboard, quick sort for solver suggestions, insertion sort for the 5 secret letters. The others are in the Sorting Lab.

**Q: Why radix sort for the dictionary?**
A: Every word is exactly 5 letters from 26, so LSD radix sort is linear, O(5 × (n + 26)), and makes zero comparisons.

**Q: Why must each radix pass be stable?**
A: So the order from earlier passes (later letters) is kept when items tie on the current letter.

**Q: Why merge sort for the leaderboard?**
A: It is stable and always O(n log n).

**Q: Why median-of-three in quick sort?**
A: The dictionary is already sorted after radix sort. A fixed pivot on sorted data gives O(n²); median-of-three avoids that.

**Q: Which elementary sort makes the fewest swaps?**
A: Selection sort, at most n − 1 (697 in our lab), though it always makes n(n−1)/2 comparisons.

**Q: Stable vs unstable?**
A: Stable keeps equal items in their original order. Bubble, insertion, merge and radix are stable; selection, comb and quick are not.

### Recursion and backtracking

**Q: Name the recursion types and an example of each.**
A: Tail (binary search), head (tree destroy: children first), tree (merge sort, Fibonacci), indirect (vowel / consonant functions calling each other), nested (McCarthy 91: `f(f(n + 11))`).

**Q: What is simulation of recursion and where is it used?**
A: Replacing recursion with your own stack: the AVL in-order walk, iterative quick sort, iterative backtracking.

**Q: What is backtracking?**
A: Build a solution step by step; when a partial solution can't work, undo the last step and try another (choose, explore, un-choose).

**Q: How does your solver prune?**
A: It drops a prefix if the colours forbid the letter there, if letter counts can't work, or if no dictionary word starts with it (checked with binary search).

**Q: How much does pruning save?**
A: Without it, 26⁵ ≈ 11.9 million words. With it, about 33,000 steps before any guess and about 13,000 after one guess.

### KMP, shuffle, two-pass

**Q: What does the LPS table store?**
A: For each position, the length of the longest proper prefix of the pattern that is also a suffix up to there. It tells KMP where to continue after a mismatch without moving back in the text.

**Q: KMP complexity vs brute force?**
A: KMP O(n + m), brute force O(n × m) worst case.

**Q: Why Fisher-Yates?**
A: Every order is equally likely, in O(n) time with no extra memory.

**Q: Explain the two-pass colouring with an example.**
A: Count the secret's letters; pass 1 marks greens and uses up counts; pass 2 marks yellow only if a count is left, else gray. APPLE vs PUPPY → yellow, gray, green, gray, gray.

**Q: Why are greens done first?**
A: So an exact match always claims its letter before a misplaced copy can. CRANE vs EERIE: one-pass wrongly shows two E's; two-pass gives gray, gray, yellow, gray, green.
