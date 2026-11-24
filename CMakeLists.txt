cmake_minimum_required(VERSION 3.16)

project(WordleDSA
    VERSION 0.1.0
    DESCRIPTION "Wordle game built on hand-implemented data structures"
    LANGUAGES CXX)

# ---------------------------------------------------------------------------
# Compiler settings
# ---------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/W4 /permissive- /utf-8 /EHsc)
else()
    add_compile_options(-Wall -Wextra -pedantic)
endif()

# ---------------------------------------------------------------------------
# Data structures and algorithms (header-only templates)
# ---------------------------------------------------------------------------
add_library(wordle_ds INTERFACE)
target_include_directories(wordle_ds INTERFACE ${CMAKE_SOURCE_DIR}/backend/include)

# ---------------------------------------------------------------------------
# Game classes (.h in backend/include, .cpp in backend/src)
# New game classes are added to this list as they are written.
# ---------------------------------------------------------------------------
add_library(wordle_core STATIC
    backend/src/Dictionary.cpp
    backend/src/WordPicker.cpp
    backend/src/Board.cpp
    backend/src/LetterTracker.cpp
    backend/src/Wordle.cpp
    backend/src/Score.cpp
    backend/src/GameSession.cpp
    backend/src/Leaderboard.cpp
    backend/src/SortingLab.cpp
    backend/src/FrequencyTable.cpp
    backend/src/Constraints.cpp
)
target_link_libraries(wordle_core PUBLIC wordle_ds)

# ---------------------------------------------------------------------------
# Server executable
# Server.cpp is the only file that includes cpp-httplib and nlohmann/json,
# so it is compiled here and not into wordle_core. The tests link only
# wordle_core and never need the HTTP headers.
# ---------------------------------------------------------------------------
add_executable(wordle_server
    backend/src/main.cpp
    backend/src/Server.cpp
)
target_include_directories(wordle_server PRIVATE ${CMAKE_SOURCE_DIR}/backend/third_party)
target_link_libraries(wordle_server PRIVATE wordle_core)
if(WIN32)
    target_link_libraries(wordle_server PRIVATE ws2_32)
endif()

# ---------------------------------------------------------------------------
# Tests (CTest)
# Each tests/<name>.cpp is its own executable and its own CTest test.
# Run with: ctest --test-dir build -C Debug --output-on-failure
# ---------------------------------------------------------------------------
enable_testing()

function(add_dsa_test name)
    add_executable(${name} tests/${name}.cpp)
    target_link_libraries(${name} PRIVATE wordle_core)
    target_include_directories(${name} PRIVATE ${CMAKE_SOURCE_DIR}/tests)
    target_compile_definitions(${name} PRIVATE WORDLE_DATA_DIR="${CMAKE_SOURCE_DIR}/data")
    add_test(NAME ${name} COMMAND ${name})
endfunction()

# Registered tests
add_dsa_test(test_safe_array)
add_dsa_test(test_linked_lists)
add_dsa_test(test_stack_queue)
add_dsa_test(test_array_stack_queue)
add_dsa_test(test_bst)
add_dsa_test(test_avl)
add_dsa_test(test_tree_properties)
add_dsa_test(test_btree)
add_dsa_test(test_hash_table)
add_dsa_test(test_min_heap)
add_dsa_test(test_search)
add_dsa_test(test_list_search)
add_dsa_test(test_sort)
add_dsa_test(test_sort_counters)
add_dsa_test(test_sorting_lab)
add_dsa_test(test_kmp)
add_dsa_test(test_shuffle)
add_dsa_test(test_dictionary)
add_dsa_test(test_board_tracker)
add_dsa_test(test_wordle)
add_dsa_test(test_score)
add_dsa_test(test_game_session)
add_dsa_test(test_leaderboard)
