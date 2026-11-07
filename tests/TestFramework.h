#ifndef WORDLE_DSA_TEST_FRAMEWORK_H
#define WORDLE_DSA_TEST_FRAMEWORK_H

// A very small unit-test helper, so the project needs no external test library.
//
// Usage:
//
//     #include "TestFramework.h"
//
//     TEST_CASE(pushBackGrowsTheArray)
//     {
//         SafeArray<int> a;
//         a.pushBack(1);
//         CHECK_EQ(a.size(), 1u);
//     }
//
//     int main() { return tf::runAll("SafeArray"); }
//
// A failed CHECK prints the file, line and expression, marks the test as
// failed and carries on, so one run reports every failure. runAll() returns
// 0 when every test passed and 1 otherwise, which is what CTest looks at.

#include <exception>
#include <iostream>

namespace tf
{
using TestFunction = void (*)();

struct TestEntry
{
    const char* name;
    TestFunction function;
};

constexpr int kMaxTests = 256;

inline TestEntry* registry()
{
    static TestEntry entries[kMaxTests];
    return entries;
}

inline int& registeredCount()
{
    static int count = 0;
    return count;
}

inline int& failuresInCurrentTest()
{
    static int failures = 0;
    return failures;
}

inline int& checksRun()
{
    static int checks = 0;
    return checks;
}

struct Registrar
{
    Registrar(const char* name, TestFunction function)
    {
        if (registeredCount() >= kMaxTests)
        {
            std::cerr << "TestFramework: too many tests, raise kMaxTests\n";
            return;
        }
        registry()[registeredCount()] = TestEntry{name, function};
        ++registeredCount();
    }
};

inline void reportFailure(const char* file, int line, const char* message)
{
    std::cout << "    " << file << ":" << line << ": " << message << "\n";
    ++failuresInCurrentTest();
}

inline int runAll(const char* suiteName)
{
    int failedTests = 0;
    std::cout << "== " << suiteName << " ==\n";

    for (int i = 0; i < registeredCount(); ++i)
    {
        const TestEntry& test = registry()[i];
        failuresInCurrentTest() = 0;

        try
        {
            test.function();
        }
        catch (const std::exception& e)
        {
            std::cout << "    unexpected exception: " << e.what() << "\n";
            ++failuresInCurrentTest();
        }
        catch (...)
        {
            std::cout << "    unexpected non-standard exception\n";
            ++failuresInCurrentTest();
        }

        if (failuresInCurrentTest() == 0)
        {
            std::cout << "[PASS] " << test.name << "\n";
        }
        else
        {
            std::cout << "[FAIL] " << test.name << "\n";
            ++failedTests;
        }
    }

    std::cout << "-- " << (registeredCount() - failedTests) << "/" << registeredCount()
              << " tests passed, " << checksRun() << " checks run --\n";
    return failedTests == 0 ? 0 : 1;
}
} // namespace tf

#define TF_CONCAT_INNER(a, b) a##b
#define TF_CONCAT(a, b) TF_CONCAT_INNER(a, b)

#define TEST_CASE(name)                                               \
    static void name();                                               \
    static ::tf::Registrar TF_CONCAT(name, _registrar)(#name, name);  \
    static void name()

#define CHECK(condition)                                                    \
    do                                                                      \
    {                                                                       \
        ++::tf::checksRun();                                                \
        if (!(condition))                                                   \
            ::tf::reportFailure(__FILE__, __LINE__, "CHECK(" #condition ")"); \
    } while (false)

#define CHECK_EQ(actual, expected)                                                    \
    do                                                                                \
    {                                                                                 \
        ++::tf::checksRun();                                                          \
        const auto& tfActual = (actual);                                              \
        const auto& tfExpected = (expected);                                          \
        if (!(tfActual == tfExpected))                                                \
        {                                                                             \
            ::tf::reportFailure(__FILE__, __LINE__, "CHECK_EQ(" #actual ", " #expected ")"); \
            std::cout << "        actual:   " << tfActual << "\n"                      \
                      << "        expected: " << tfExpected << "\n";                   \
        }                                                                             \
    } while (false)

// Passes only if the expression throws exceptionType (or a class derived from it).
#define CHECK_THROWS(expression, exceptionType)                                       \
    do                                                                                \
    {                                                                                 \
        ++::tf::checksRun();                                                          \
        bool tfThrewExpected = false;                                                 \
        try                                                                           \
        {                                                                             \
            (void)(expression);                                                       \
        }                                                                             \
        catch (const exceptionType&)                                                  \
        {                                                                             \
            tfThrewExpected = true;                                                   \
        }                                                                             \
        catch (...)                                                                   \
        {                                                                             \
        }                                                                             \
        if (!tfThrewExpected)                                                         \
            ::tf::reportFailure(__FILE__, __LINE__,                                   \
                                "CHECK_THROWS(" #expression ", " #exceptionType ")"); \
    } while (false)

#endif // WORDLE_DSA_TEST_FRAMEWORK_H
