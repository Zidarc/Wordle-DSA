# Contributing to Wordle-DSA

## 1. Team Members

| Roll No. | Name |
|----------|------|
| 25K-2513 | Ali Hussain |
| 25K-3011 | Ali |
| 25K-0906 | M. Abbas |

Daily rotation order: **Ali Hussain → Ali → M. Abbas**, repeating. See the [16-Day Schedule](README.md#16-16-day-schedule) in the README for who owns which day.

---

## 2. Branch Structure

Wordle-DSA uses:

```
main
develop
feature/*
fix/*
docs/*
```

### `main`
Stable, tested, submission-ready code. Only updated from `develop` at a release point (final merge on Day 16).

### `develop`
The integrated team development branch. All Pull Requests target `develop`.

### `feature/*`
New functionality. Examples:

```
feature/project-setup
feature/linked-lists
feature/trees
feature/hash-heap
feature/algorithms
feature/dictionary
feature/wordle-core
feature/session-leaderboard
feature/server-api
feature/frontend-board
feature/frontend-controls
feature/frontend-leaderboard
feature/integration
feature/ui-polish
```

### `fix/*`
Bug fixes. Examples:

```
fix/duplicate-letter-feedback
fix/empty-stack-pop
fix/leaderboard-missing-file
```

### `docs/*`
Documentation-only changes. Examples:

```
docs/api
docs/complexity
docs/architecture
```

---

## 3. Standard Workflow

Start from the latest `develop`:

```bash
git switch develop
git pull origin develop
```

Create a feature branch:

```bash
git switch -c feature/your-feature
```

Work and test locally.

Check your changes:

```bash
git status
git diff
```

Stage only the files you intended to change:

```bash
git add path/to/file.cpp path/to/file.h
```

Commit:

```bash
git commit -m "Describe the change"
```

Push:

```bash
git push -u origin feature/your-feature
```

Open a Pull Request:

```
feature/your-feature  →  develop
```

Another team member reviews the Pull Request. After approval and successful testing, merge it into `develop`.

---

## 4. Before Starting New Work

Always sync your local `develop` first:

```bash
git switch develop
git pull origin develop
```

Then create the new branch.

This keeps you from building on an outdated version of the project and reduces merge conflicts.

---

## 5. Pull Request Requirements

A Pull Request must have:

- A clear title.
- A description of the changes.
- A list of the files changed.
- The tests performed.
- Any known issues.
- Handoff information for the next developer.

Example:

```
Title:
Implement BST and AVL tree templates

Changes:
- Added BST<T> with insert, contains, destroy (recursive).
- Added AVLTree<T> with insert, LL/RR/LR/RL rotations, contains, height.
- Added iterative in-order traversal using Stack<T>.
- Rule of Three for both trees (deep copy).
- Added tests/test_bst.cpp and tests/test_avl.cpp.

Files changed:
- backend/include/ds/BST.h
- backend/include/ds/AVLTree.h
- tests/test_bst.cpp
- tests/test_avl.cpp
- CMakeLists.txt (registered tests)

Testing:
- Empty tree, single element
- Search present / missing (char and string)
- All four AVL rotations
- Height stays O(log n) after inserting 1000 sorted values
- In-order traversal is sorted
- Copy is independent of the original

Known issues:
- None

Handoff:
- Dictionary (Day 6) can build AVLTree<std::string> by inserting the sorted words.
- contains() takes an optional int& counter for the DSA Insights comparison count.
```

### Review checklist

The reviewer checks that:

- [ ] The project builds (`cmake --build build`, and `npm run build` for frontend changes).
- [ ] All tests pass (`ctest --test-dir build -C Debug`).
- [ ] The change matches the day's task and does not touch unrelated files.
- [ ] Header and source files are separated (templates are the only exception).
- [ ] Classes that own dynamic memory follow the Rule of Three and don't leak.
- [ ] No STL containers or algorithms replace our own data structures.
- [ ] No build output, `node_modules/` or secrets are included.

---

## 6. Coding Rules

### General
- Do not develop directly on `main`.
- Do not commit directly to `develop`. Use a Pull Request.
- Never commit build files (`build/`, `*.exe`, `*.obj`, `node_modules/`, `frontend/dist/`).
- Never commit passwords, tokens or `.env` files.
- Do not change unrelated files.
- Do not change another member's code without discussing it with them first.
- Test your changes before opening a Pull Request.
- Do not add features outside the agreed scope without team agreement.

### C++ backend
- Use C++17.
- Keep declarations in `.h` files and implementations in `.cpp` files. **Exception:** the templates in `include/ds/` and `include/algorithms/` live entirely in their header.
- Use header guards (`#ifndef ... #define ... #endif`) or `#pragma once`.
- **Every data structure and algorithm is implemented by hand.** Do not use `std::vector`, `std::list`, `std::stack`, `std::queue`, `std::priority_queue`, `std::set`, `std::map`, `std::unordered_map`, `std::sort` or `std::binary_search` in project code. Use our own classes from `include/ds/` and `include/algorithms/` instead.
- `std::string`, `<iostream>`, `<fstream>`, `<stdexcept>`, `<random>` and `<chrono>` are allowed.
- `httplib.h` and `json.hpp` are used **only** in `Server.cpp`, to convert between our data structures and JSON/HTTP.
- **Rule of Three:** every class that owns dynamic memory implements a destructor, copy constructor and copy assignment operator (with a self-assignment check).
- Free every `new` with a matching `delete`. Use `delete[]` for arrays.
- Out-of-range access in `SafeArray` throws `std::out_of_range`. Do not ignore it silently.
- Search functions take an optional comparison counter so the DSA Insights panel can report it.
- Keep `main.cpp` small. It only starts the server.
- Use meaningful names: `isInDictionary()`, not `chk()`.
- Keep functions reasonably small.
- The only external libraries are `httplib.h` and `json.hpp` in `backend/third_party/`. Do not add others without team agreement.

### React frontend
- Use function components and hooks.
- Put all backend calls in `src/api/client.js`. Components never call `fetch` directly.
- One component per file in `src/components/`.
- **No game logic in the frontend.** The frontend never decides colours or validates words. It only displays what the backend returns.
- Do not add npm packages without team agreement.

---

## 7. Commit Message Style

Use descriptive, imperative messages:

```
Add project folder structure and CMake skeleton
Implement bounds-checked SafeArray with Rule of Three
Implement singly, doubly and circular linked lists
Implement stack and queue on linked lists
Implement AVL tree with rotations
Implement hash table with chaining and rehashing
Implement radix sort for fixed-length words
Add interpolation search on base-26 word keys
Implement two-pass guess comparison for repeated letters
Add hint queue and Hard-mode hint check
Implement top-K leaderboard with min-heap
Add HTTP routes for new game and guess
Add Board and Tile components
Add DSA Insights panel
```

Avoid:

```
Update
Done
Final
Changes
test
fix
asdf
```

---

## 8. Build Before Pull Request

The project must build and pass its tests before you request a review.

### Backend

```bash
cmake -B build
cmake --build build
ctest --test-dir build -C Debug --output-on-failure
```

### Frontend (if you changed anything in `frontend/`)

```bash
cd frontend
npm install
npm run build
```

---

## 9. Merge Conflicts

If a merge conflict happens:

1. Do not delete someone else's work without checking it.
2. Read both versions.
3. Decide which code should stay. Ask the other author if you are unsure.
4. Remove the Git conflict markers (`<<<<<<<`, `=======`, `>>>>>>>`).
5. Build and test again.
6. Commit the resolution.

Useful commands:

```bash
git status
git add <resolved-files>
git commit -m "Resolve merge conflict in <file>"
git push
```

---

## 10. Daily Handoff

At the end of each day, the owner adds a handoff note to the Pull Request description **and** posts it in the team group chat:

```
DAY:
DATE:
OWNER:

COMPLETED:

FILES CHANGED:

TESTS PERFORMED:

KNOWN ISSUES:

NEXT DEVELOPER:

NEXT TASK:

NOTES FOR NEXT DEVELOPER:
```

Example:

```
DAY: 2
DATE: Thu 5 Nov 2026
OWNER: Ali

COMPLETED:
- SinglyLinkedList, DoublyLinkedList, CircularLinkedList
- Stack and Queue built on SinglyLinkedList
- Rule of Three for all five

FILES CHANGED:
- backend/include/ds/SinglyLinkedList.h
- backend/include/ds/DoublyLinkedList.h
- backend/include/ds/CircularLinkedList.h
- backend/include/ds/Stack.h
- backend/include/ds/Queue.h
- tests/test_linked_lists.cpp
- tests/test_stack_queue.cpp
- CMakeLists.txt

TESTS PERFORMED:
- test_linked_lists: all cases pass
- test_stack_queue: all cases pass

KNOWN ISSUES:
- None

NEXT DEVELOPER: M. Abbas
NEXT TASK: Day 3 - BST and AVL tree

NOTES FOR NEXT DEVELOPER:
- Use Stack<T> for the iterative in-order traversal of the AVL tree.
```

If a task is not finished, say so clearly. The next owner finishes it before starting their own task.

---

## 11. Scope Protection

Wordle-DSA is a **Data Structures** project. The data structures are the focus, not the technology stack.

Do not add:

- Online multiplayer
- User accounts or authentication
- Databases
- Online deployment / hosting
- Mobile applications
- AI / automatic solvers
- External dictionary or word APIs
- Large frontend UI libraries or frameworks beyond React + Vite
- Extra data structures that don't serve a real game feature (see [Course Topic Coverage](README.md#7-course-topic-coverage))

unless the whole team, and the instructor where needed, agrees to the scope change.

The core project must stay stable.
