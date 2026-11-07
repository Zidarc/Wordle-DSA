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
# Server executable
# Game classes (.cpp files in backend/src) are added here as they are written.
# ---------------------------------------------------------------------------
add_executable(wordle_server
    backend/src/main.cpp
)
target_link_libraries(wordle_server PRIVATE wordle_ds)

# ---------------------------------------------------------------------------
# Tests (CTest)
# Each tests/<name>.cpp is its own executable and its own CTest test.
# Run with: ctest --test-dir build -C Debug --output-on-failure
# ---------------------------------------------------------------------------
enable_testing()

function(add_dsa_test name)
    add_executable(${name} tests/${name}.cpp)
    target_link_libraries(${name} PRIVATE wordle_ds)
    target_include_directories(${name} PRIVATE ${CMAKE_SOURCE_DIR}/tests)
    add_test(NAME ${name} COMMAND ${name})
endfunction()

# Registered tests
add_dsa_test(test_safe_array)
