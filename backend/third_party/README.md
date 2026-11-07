# Third-party headers

These are the only external libraries in the project. Both are single-header,
MIT-licensed, and used **only** in `backend/src/Server.cpp` to turn our own
data structures into HTTP responses and JSON. They are never used inside the
data structures or the game logic.

| File | Library | Version | Source | License |
|------|---------|---------|--------|---------|
| `httplib.h` | cpp-httplib | 0.18.1 | https://github.com/yhirose/cpp-httplib/releases/tag/v0.18.1 | MIT |
| `json.hpp` | nlohmann/json | 3.11.3 | https://github.com/nlohmann/json/releases/tag/v3.11.3 | MIT |

Do not edit these files. To upgrade, replace the whole file with a newer
release and update this table.
