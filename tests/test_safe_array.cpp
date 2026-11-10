#include "TestFramework.h"
#include "ds/SafeArray.h"

#include <stdexcept>
#include <string>

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

TEST_CASE(defaultConstructedArrayIsEmpty)
{
    SafeArray<int> a;
    CHECK(a.empty());
    CHECK_EQ(a.size(), 0u);
    CHECK_EQ(a.capacity(), 0u);
}

TEST_CASE(sizedConstructorFillsEveryElement)
{
    SafeArray<int> a(5, 7);
    CHECK_EQ(a.size(), 5u);
    for (std::size_t i = 0; i < a.size(); ++i)
        CHECK_EQ(a[i], 7);
}

TEST_CASE(sizedConstructorDefaultsToValueInitialised)
{
    SafeArray<std::string> words(3);
    CHECK_EQ(words.size(), 3u);
    CHECK_EQ(words[2], std::string());
}

// ---------------------------------------------------------------------------
// Element access and bounds checking
// ---------------------------------------------------------------------------

TEST_CASE(validAccessReadsAndWrites)
{
    SafeArray<int> a(3);
    a[0] = 10;
    a[1] = 20;
    a.at(2) = 30;
    CHECK_EQ(a[0], 10);
    CHECK_EQ(a.at(1), 20);
    CHECK_EQ(a[2], 30);
    CHECK_EQ(a.front(), 10);
    CHECK_EQ(a.back(), 30);
}

TEST_CASE(outOfRangeAccessThrows)
{
    SafeArray<int> a(3);
    CHECK_THROWS(a[3], std::out_of_range);
    CHECK_THROWS(a.at(100), std::out_of_range);
    CHECK_THROWS(a[static_cast<std::size_t>(-1)], std::out_of_range);
}

TEST_CASE(constAccessIsAlsoChecked)
{
    const SafeArray<int> a(2, 4);
    CHECK_EQ(a[1], 4);
    CHECK_THROWS(a[2], std::out_of_range);
}

TEST_CASE(emptyArrayAccessThrows)
{
    SafeArray<int> a;
    CHECK_THROWS(a[0], std::out_of_range);
    CHECK_THROWS(a.front(), std::out_of_range);
    CHECK_THROWS(a.back(), std::out_of_range);
    CHECK_THROWS(a.popBack(), std::out_of_range);
}

TEST_CASE(outOfRangeMessageNamesTheIndex)
{
    SafeArray<int> a(2);
    try
    {
        a[5] = 1;
        CHECK(false); // should not get here
    }
    catch (const std::out_of_range& e)
    {
        CHECK(std::string(e.what()).find("5") != std::string::npos);
    }
}

// ---------------------------------------------------------------------------
// pushBack / popBack / growth
// ---------------------------------------------------------------------------

TEST_CASE(pushBackGrowsPastInitialCapacity)
{
    SafeArray<int> a;
    for (int i = 0; i < 1000; ++i)
        a.pushBack(i);

    CHECK_EQ(a.size(), 1000u);
    CHECK(a.capacity() >= 1000u);
    for (int i = 0; i < 1000; ++i)
        CHECK_EQ(a[static_cast<std::size_t>(i)], i);
}

TEST_CASE(popBackRemovesLastElement)
{
    SafeArray<int> a;
    a.pushBack(1);
    a.pushBack(2);
    a.popBack();
    CHECK_EQ(a.size(), 1u);
    CHECK_EQ(a.back(), 1);
    CHECK_THROWS(a[1], std::out_of_range);
}

// ---------------------------------------------------------------------------
// insertAt / removeAt / swapAt
// ---------------------------------------------------------------------------

TEST_CASE(insertAtStartMiddleAndEnd)
{
    SafeArray<int> a;
    a.pushBack(2);
    a.pushBack(4);
    a.insertAt(0, 1); // 1 2 4
    a.insertAt(2, 3); // 1 2 3 4
    a.insertAt(4, 5); // 1 2 3 4 5 (index == size appends)

    CHECK_EQ(a.size(), 5u);
    for (std::size_t i = 0; i < a.size(); ++i)
        CHECK_EQ(a[i], static_cast<int>(i) + 1);

    CHECK_THROWS(a.insertAt(7, 9), std::out_of_range);
}

TEST_CASE(removeAtShiftsElementsLeft)
{
    SafeArray<int> a;
    for (int i = 1; i <= 5; ++i)
        a.pushBack(i);

    a.removeAt(0); // 2 3 4 5
    a.removeAt(1); // 2 4 5
    a.removeAt(2); // 2 4

    CHECK_EQ(a.size(), 2u);
    CHECK_EQ(a[0], 2);
    CHECK_EQ(a[1], 4);
    CHECK_THROWS(a.removeAt(2), std::out_of_range);
}

TEST_CASE(swapAtExchangesTwoElements)
{
    SafeArray<std::string> a;
    a.pushBack("CRANE");
    a.pushBack("SLATE");
    a.swapAt(0, 1);
    CHECK_EQ(a[0], std::string("SLATE"));
    CHECK_EQ(a[1], std::string("CRANE"));
    a.swapAt(1, 1);
    CHECK_EQ(a[1], std::string("CRANE"));
    CHECK_THROWS(a.swapAt(0, 2), std::out_of_range);
}

// ---------------------------------------------------------------------------
// resize / reserve / clear
// ---------------------------------------------------------------------------

TEST_CASE(resizeLargerKeepsExistingElements)
{
    SafeArray<int> a;
    a.pushBack(1);
    a.pushBack(2);
    a.resize(5, 9);

    CHECK_EQ(a.size(), 5u);
    CHECK_EQ(a[0], 1);
    CHECK_EQ(a[1], 2);
    CHECK_EQ(a[2], 9);
    CHECK_EQ(a[4], 9);
}

TEST_CASE(resizeSmallerDropsTrailingElements)
{
    SafeArray<int> a(5, 3);
    a.resize(2);
    CHECK_EQ(a.size(), 2u);
    CHECK_THROWS(a[2], std::out_of_range);

    a.resize(4); // grow again: new slots are value-initialised, not old data
    CHECK_EQ(a[2], 0);
    CHECK_EQ(a[3], 0);
}

TEST_CASE(reserveDoesNotChangeSize)
{
    SafeArray<int> a;
    a.pushBack(42);
    a.reserve(100);
    CHECK_EQ(a.size(), 1u);
    CHECK(a.capacity() >= 100u);
    CHECK_EQ(a[0], 42);

    a.reserve(10); // never shrinks
    CHECK(a.capacity() >= 100u);
}

TEST_CASE(clearEmptiesTheArray)
{
    SafeArray<int> a(10, 1);
    a.clear();
    CHECK(a.empty());
    CHECK_EQ(a.capacity(), 0u);
    a.pushBack(5); // still usable after clear
    CHECK_EQ(a[0], 5);
}

// ---------------------------------------------------------------------------
// Rule of Three: deep copies
// ---------------------------------------------------------------------------

TEST_CASE(copyConstructorMakesDeepCopy)
{
    SafeArray<int> original;
    original.pushBack(1);
    original.pushBack(2);

    SafeArray<int> copy(original);
    copy[0] = 100;
    copy.pushBack(3);

    CHECK_EQ(original.size(), 2u);
    CHECK_EQ(original[0], 1);
    CHECK_EQ(copy.size(), 3u);
    CHECK_EQ(copy[0], 100);
}

TEST_CASE(copyOfEmptyArrayIsEmpty)
{
    SafeArray<int> empty;
    SafeArray<int> copy(empty);
    CHECK(copy.empty());
    copy.pushBack(1);
    CHECK(empty.empty());
}

TEST_CASE(copyAssignmentMakesDeepCopy)
{
    SafeArray<std::string> a;
    a.pushBack("APPLE");

    SafeArray<std::string> b;
    b.pushBack("X");
    b.pushBack("Y");
    b.pushBack("Z");

    b = a;
    CHECK_EQ(b.size(), 1u);
    CHECK_EQ(b[0], std::string("APPLE"));

    b[0] = "PLUMB";
    CHECK_EQ(a[0], std::string("APPLE"));
}

TEST_CASE(selfAssignmentLeavesArrayUnchanged)
{
    SafeArray<int> a;
    a.pushBack(4);
    a.pushBack(8);

    SafeArray<int>& alias = a;
    a = alias;

    CHECK_EQ(a.size(), 2u);
    CHECK_EQ(a[0], 4);
    CHECK_EQ(a[1], 8);
}

// ---------------------------------------------------------------------------
// Iteration
// ---------------------------------------------------------------------------

TEST_CASE(rangeForVisitsEveryElementInOrder)
{
    SafeArray<int> a;
    for (int i = 1; i <= 4; ++i)
        a.pushBack(i);

    int sum = 0;
    int expectedNext = 1;
    for (int value : a)
    {
        CHECK_EQ(value, expectedNext);
        ++expectedNext;
        sum += value;
    }
    CHECK_EQ(sum, 10);

    SafeArray<int> empty;
    int visits = 0;
    for (int value : empty)
    {
        (void)value;
        ++visits;
    }
    CHECK_EQ(visits, 0);
}

int main()
{
    return tf::runAll("SafeArray");
}
