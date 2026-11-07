#include "TestFramework.h"
#include "ds/CircularLinkedList.h"
#include "ds/DoublyLinkedList.h"
#include "ds/SinglyLinkedList.h"

#include <cstddef>
#include <stdexcept>
#include <string>

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

// Counts how many Tracked objects are alive, so tests can prove that
// clear(), destructors and removals free every node (no leaks).
struct Tracked
{
    static int alive;
    int value;

    Tracked(int v = 0) : value(v) { ++alive; }
    Tracked(const Tracked& other) : value(other.value) { ++alive; }
    Tracked& operator=(const Tracked& other) = default;
    ~Tracked() { --alive; }

    bool operator==(const Tracked& other) const { return value == other.value; }
};
int Tracked::alive = 0;

// True if iterating `list` front to back yields exactly expected[0..count).
template <typename List>
bool contentsAre(const List& list, const int* expected, std::size_t count)
{
    if (list.size() != count)
        return false;
    std::size_t i = 0;
    for (const int& value : list)
    {
        if (i >= count || value != expected[i])
            return false;
        ++i;
    }
    return i == count;
}

template <typename List>
void fillWith(List& list, int from, int to)
{
    for (int v = from; v <= to; ++v)
        list.pushBack(v);
}

// ===========================================================================
// SinglyLinkedList
// ===========================================================================

TEST_CASE(singlyEmptyList)
{
    SinglyLinkedList<int> list;
    CHECK(list.empty());
    CHECK_EQ(list.size(), 0u);
    CHECK(list.begin() == list.end());
    CHECK_THROWS(list.front(), std::out_of_range);
    CHECK_THROWS(list.back(), std::out_of_range);
    CHECK_THROWS(list.popFront(), std::out_of_range);
    CHECK_THROWS(list.popBack(), std::out_of_range);
    CHECK_THROWS(list.at(0), std::out_of_range);
    CHECK(!list.removeFirst(1));
}

TEST_CASE(singlyPushFrontAndBack)
{
    SinglyLinkedList<int> list;
    list.pushBack(2);
    list.pushFront(1);
    list.pushBack(3);

    const int expected[] = {1, 2, 3};
    CHECK(contentsAre(list, expected, 3));
    CHECK_EQ(list.front(), 1);
    CHECK_EQ(list.back(), 3);
}

TEST_CASE(singlyPopFrontAndBack)
{
    SinglyLinkedList<int> list;
    fillWith(list, 1, 4);

    list.popFront(); // 2 3 4
    list.popBack();  // 2 3
    const int expected[] = {2, 3};
    CHECK(contentsAre(list, expected, 2));
    CHECK_EQ(list.back(), 3);

    list.popBack();
    list.popBack();
    CHECK(list.empty());
    list.pushBack(9); // head and tail were both reset correctly
    CHECK_EQ(list.front(), 9);
    CHECK_EQ(list.back(), 9);
}

TEST_CASE(singlyInsertAtHeadMiddleTail)
{
    SinglyLinkedList<int> list;
    list.insertAt(0, 3); // 3
    list.insertAt(0, 1); // 1 3
    list.insertAt(1, 2); // 1 2 3
    list.insertAt(3, 4); // 1 2 3 4

    const int expected[] = {1, 2, 3, 4};
    CHECK(contentsAre(list, expected, 4));
    CHECK_EQ(list.back(), 4);
    CHECK_THROWS(list.insertAt(9, 0), std::out_of_range);
}

TEST_CASE(singlyRemoveAtHeadMiddleTail)
{
    SinglyLinkedList<int> list;
    fillWith(list, 1, 5);

    list.removeAt(0); // 2 3 4 5
    list.removeAt(1); // 2 4 5
    list.removeAt(2); // 2 4   (tail removed)

    const int expected[] = {2, 4};
    CHECK(contentsAre(list, expected, 2));
    CHECK_EQ(list.back(), 4);
    list.pushBack(6); // tail pointer must have been updated
    CHECK_EQ(list.back(), 6);
    CHECK_THROWS(list.removeAt(3), std::out_of_range);
}

TEST_CASE(singlyAtAndContains)
{
    SinglyLinkedList<std::string> list;
    list.pushBack("BRAVE");
    list.pushBack("CRANE");
    list.pushBack("SLATE");

    CHECK_EQ(list.at(1), std::string("CRANE"));
    CHECK(list.contains("SLATE"));
    CHECK(!list.contains("PLUMB"));
    CHECK_THROWS(list.at(3), std::out_of_range);
}

TEST_CASE(singlyRemoveFirstMatchOnly)
{
    SinglyLinkedList<int> list;
    list.pushBack(7);
    list.pushBack(8);
    list.pushBack(7);
    list.pushBack(9);

    CHECK(list.removeFirst(7)); // 8 7 9
    const int expected[] = {8, 7, 9};
    CHECK(contentsAre(list, expected, 3));

    CHECK(list.removeFirst(9)); // tail
    CHECK_EQ(list.back(), 7);
    CHECK(!list.removeFirst(42));
}

// The hash table (Day 4) stores key/value entries in these lists and looks
// them up by key with findIf / removeIf.
struct Entry
{
    std::string key;
    int value;
};

TEST_CASE(singlyFindIfAndRemoveIfByKey)
{
    SinglyLinkedList<Entry> chain;
    chain.pushBack(Entry{"apple", 1});
    chain.pushBack(Entry{"crane", 2});
    chain.pushBack(Entry{"slate", 3});

    Entry* found = chain.findIf([](const Entry& e) { return e.key == "crane"; });
    CHECK(found != nullptr);
    if (found != nullptr)
    {
        found->value = 20; // update in place
        CHECK_EQ(chain.at(1).value, 20);
    }

    CHECK(chain.findIf([](const Entry& e) { return e.key == "zebra"; }) == nullptr);
    CHECK(chain.removeIf([](const Entry& e) { return e.key == "apple"; }));
    CHECK_EQ(chain.size(), 2u);
    CHECK_EQ(chain.front().key, std::string("crane"));
}

TEST_CASE(singlyCopyConstructorIsDeep)
{
    SinglyLinkedList<int> original;
    fillWith(original, 1, 3);

    SinglyLinkedList<int> copy(original);
    copy.front() = 100;
    copy.pushBack(4);

    const int expectedOriginal[] = {1, 2, 3};
    CHECK(contentsAre(original, expectedOriginal, 3));
    CHECK_EQ(copy.size(), 4u);
    CHECK_EQ(copy.front(), 100);
}

TEST_CASE(singlyCopyAssignmentIsDeepAndHandlesSelf)
{
    SinglyLinkedList<int> a;
    fillWith(a, 1, 3);
    SinglyLinkedList<int> b;
    fillWith(b, 7, 12);

    b = a;
    const int expected[] = {1, 2, 3};
    CHECK(contentsAre(b, expected, 3));
    b.popFront();
    CHECK(contentsAre(a, expected, 3));

    SinglyLinkedList<int>& alias = a;
    a = alias;
    CHECK(contentsAre(a, expected, 3));
}

TEST_CASE(singlyFreesEveryNode)
{
    Tracked::alive = 0;
    {
        SinglyLinkedList<Tracked> list;
        for (int i = 0; i < 10; ++i)
            list.pushBack(Tracked(i));
        CHECK_EQ(Tracked::alive, 10);

        list.popFront();
        list.popBack();
        list.removeAt(3);
        CHECK_EQ(Tracked::alive, 7);

        SinglyLinkedList<Tracked> copy(list);
        CHECK_EQ(Tracked::alive, 14);
        copy.clear();
        CHECK_EQ(Tracked::alive, 7);
    }
    CHECK_EQ(Tracked::alive, 0); // destructor freed the rest
}

// ===========================================================================
// DoublyLinkedList
// ===========================================================================

TEST_CASE(doublyEmptyList)
{
    DoublyLinkedList<int> list;
    CHECK(list.empty());
    CHECK(list.begin() == list.end());
    CHECK_THROWS(list.front(), std::out_of_range);
    CHECK_THROWS(list.back(), std::out_of_range);
    CHECK_THROWS(list.popFront(), std::out_of_range);
    CHECK_THROWS(list.popBack(), std::out_of_range);
    CHECK_THROWS(list.removeAt(0), std::out_of_range);
}

TEST_CASE(doublyPushAndPopBothEnds)
{
    DoublyLinkedList<int> list;
    list.pushBack(2);
    list.pushFront(1);
    list.pushBack(3);

    const int expected[] = {1, 2, 3};
    CHECK(contentsAre(list, expected, 3));

    list.popBack();
    list.popFront();
    CHECK_EQ(list.size(), 1u);
    CHECK_EQ(list.front(), 2);
    CHECK_EQ(list.back(), 2);

    list.popBack();
    CHECK(list.empty());
    list.pushFront(5);
    CHECK_EQ(list.back(), 5);
}

TEST_CASE(doublyForwardTraversal)
{
    DoublyLinkedList<int> list;
    fillWith(list, 1, 5);
    const int expected[] = {1, 2, 3, 4, 5};
    CHECK(contentsAre(list, expected, 5));
}

TEST_CASE(doublyBackwardTraversalFromEnd)
{
    DoublyLinkedList<int> list;
    fillWith(list, 1, 5);

    int expected = 5;
    auto it = list.end();
    while (it != list.begin())
    {
        --it;
        CHECK_EQ(*it, expected);
        --expected;
    }
    CHECK_EQ(expected, 0);
}

TEST_CASE(doublyReplayStyleStepping)
{
    DoublyLinkedList<std::string> guesses;
    guesses.pushBack("CRANE");
    guesses.pushBack("SLOTH");
    guesses.pushBack("PLUMB");

    auto step = guesses.begin();
    CHECK_EQ(*step, std::string("CRANE"));
    ++step;
    CHECK_EQ(*step, std::string("SLOTH"));
    ++step;
    CHECK_EQ(*step, std::string("PLUMB"));
    --step;
    CHECK_EQ(*step, std::string("SLOTH")); // step back
    --step;
    CHECK_EQ(*step, std::string("CRANE"));
}

TEST_CASE(doublyInsertAtAndRemoveAtMiddle)
{
    DoublyLinkedList<int> list;
    fillWith(list, 1, 6);

    list.insertAt(3, 99); // 1 2 3 99 4 5 6
    CHECK_EQ(list.at(3), 99);
    CHECK_EQ(list.at(4), 4);

    list.removeAt(3);     // 1 2 3 4 5 6
    list.removeAt(4);     // 1 2 3 4 6   (index in the back half)
    list.removeAt(0);     // 2 3 4 6
    list.removeAt(3);     // 2 3 4
    const int expected[] = {2, 3, 4};
    CHECK(contentsAre(list, expected, 3));

    // Links in both directions must still be consistent.
    auto it = list.end();
    --it;
    CHECK_EQ(*it, 4);
    --it;
    CHECK_EQ(*it, 3);
    --it;
    CHECK_EQ(*it, 2);
    CHECK(it == list.begin());
}

TEST_CASE(doublyAtWalksFromNearerEnd)
{
    DoublyLinkedList<int> list;
    fillWith(list, 0, 99);
    for (std::size_t i = 0; i < 100; ++i)
        CHECK_EQ(list.at(i), static_cast<int>(i));
    CHECK_THROWS(list.at(100), std::out_of_range);
}

TEST_CASE(doublyRemoveFirstAndContains)
{
    DoublyLinkedList<int> list;
    fillWith(list, 1, 3);
    CHECK(list.contains(2));
    CHECK(list.removeFirst(3));
    CHECK_EQ(list.back(), 2);
    CHECK(!list.contains(3));
    CHECK(!list.removeFirst(3));
}

TEST_CASE(doublyCopyIsDeepAndHandlesSelf)
{
    DoublyLinkedList<int> original;
    fillWith(original, 1, 3);

    DoublyLinkedList<int> copy(original);
    copy.back() = 30;
    const int expected[] = {1, 2, 3};
    CHECK(contentsAre(original, expected, 3));

    DoublyLinkedList<int> assigned;
    assigned.pushBack(42);
    assigned = original;
    assigned.popFront();
    CHECK(contentsAre(original, expected, 3));
    CHECK_EQ(assigned.size(), 2u);

    // The copy must be traversable backwards too (prev links copied correctly).
    auto it = copy.end();
    --it;
    CHECK_EQ(*it, 30);
    --it;
    CHECK_EQ(*it, 2);

    DoublyLinkedList<int>& alias = original;
    original = alias;
    CHECK(contentsAre(original, expected, 3));
}

TEST_CASE(doublyFreesEveryNode)
{
    Tracked::alive = 0;
    {
        DoublyLinkedList<Tracked> list;
        for (int i = 0; i < 10; ++i)
            list.pushBack(Tracked(i));
        list.popFront();
        list.popBack();
        list.removeAt(4);
        CHECK_EQ(Tracked::alive, 7);

        DoublyLinkedList<Tracked> other;
        other.pushBack(Tracked(1));
        other = list; // the old node in `other` must be freed
        CHECK_EQ(Tracked::alive, 14);
    }
    CHECK_EQ(Tracked::alive, 0);
}

// ===========================================================================
// CircularLinkedList
// ===========================================================================

TEST_CASE(circularEmptyList)
{
    CircularLinkedList<int> list;
    CHECK(list.empty());
    CHECK(list.begin() == list.end());
    CHECK(!list.cursorAtStart());
    CHECK_THROWS(list.front(), std::out_of_range);
    CHECK_THROWS(list.current(), std::out_of_range);
    CHECK_THROWS(list.advance(), std::out_of_range);
    CHECK_THROWS(list.popFront(), std::out_of_range);
    CHECK(!list.removeFirst(1));
}

TEST_CASE(circularSingleNodePointsToItself)
{
    CircularLinkedList<int> list;
    list.pushBack(7);
    CHECK_EQ(list.front(), 7);
    CHECK_EQ(list.back(), 7);
    CHECK_EQ(list.current(), 7);
    CHECK_EQ(list.advance(), 7); // wraps straight back to itself
    CHECK(list.cursorAtStart());
}

TEST_CASE(circularPushFrontAndBackOrder)
{
    CircularLinkedList<int> list;
    list.pushBack(2);
    list.pushBack(3);
    list.pushFront(1);

    const int expected[] = {1, 2, 3};
    CHECK(contentsAre(list, expected, 3));
    CHECK_EQ(list.front(), 1);
    CHECK_EQ(list.back(), 3);
}

TEST_CASE(circularIterationIsExactlyOneLap)
{
    CircularLinkedList<int> list;
    fillWith(list, 1, 4);

    int visits = 0;
    for (const int& value : list)
    {
        (void)value;
        ++visits;
    }
    CHECK_EQ(visits, 4);
}

TEST_CASE(circularAdvanceWrapsAfterLastElement)
{
    CircularLinkedList<std::string> words;
    words.pushBack("BRAVE");
    words.pushBack("CRANE");
    words.pushBack("SLATE");

    CHECK_EQ(words.current(), std::string("BRAVE"));
    CHECK(words.cursorAtStart());
    CHECK_EQ(words.advance(), std::string("CRANE"));
    CHECK(!words.cursorAtStart());
    CHECK_EQ(words.advance(), std::string("SLATE"));
    CHECK_EQ(words.advance(), std::string("BRAVE")); // wrapped around
    CHECK(words.cursorAtStart());                   // full lap completed
}

TEST_CASE(circularNoRepeatWithinOneCycle)
{
    // WordPicker pattern: take current, then advance. Over one full lap
    // every word must be handed out exactly once.
    CircularLinkedList<int> words;
    fillWith(words, 0, 9);

    int timesSeen[10] = {0};
    for (int game = 0; game < 10; ++game)
    {
        ++timesSeen[words.current()];
        words.advance();
    }
    for (int i = 0; i < 10; ++i)
        CHECK_EQ(timesSeen[i], 1);
    CHECK(words.cursorAtStart());
}

TEST_CASE(circularResetCursor)
{
    CircularLinkedList<int> list;
    fillWith(list, 1, 3);
    list.advance();
    list.advance();
    CHECK_EQ(list.current(), 3);
    list.resetCursor();
    CHECK_EQ(list.current(), 1);
    CHECK(list.cursorAtStart());
}

TEST_CASE(circularPopFrontKeepsCycleAndCursorValid)
{
    CircularLinkedList<int> list;
    fillWith(list, 1, 3); // cursor on 1 (the head)

    list.popFront(); // 2 3, cursor moves to 2
    CHECK_EQ(list.front(), 2);
    CHECK_EQ(list.current(), 2);
    CHECK_EQ(list.advance(), 3);
    CHECK_EQ(list.advance(), 2); // still a cycle

    list.popFront();
    list.popFront();
    CHECK(list.empty());
    list.pushBack(5);
    CHECK_EQ(list.current(), 5);
}

TEST_CASE(circularRemoveFirstUpdatesTailAndCursor)
{
    CircularLinkedList<int> list;
    fillWith(list, 1, 4);
    list.advance();
    list.advance(); // cursor on 3

    CHECK(list.removeFirst(3)); // cursor moves on to 4
    CHECK_EQ(list.current(), 4);

    CHECK(list.removeFirst(4)); // removing the tail; cursor wraps to 1
    CHECK_EQ(list.back(), 2);
    CHECK_EQ(list.current(), 1);

    const int expected[] = {1, 2};
    CHECK(contentsAre(list, expected, 2));
    CHECK(!list.removeFirst(99));

    list.pushBack(5); // tail pointer must be right for this to land at the end
    const int expectedAfter[] = {1, 2, 5};
    CHECK(contentsAre(list, expectedAfter, 3));
}

TEST_CASE(circularContains)
{
    CircularLinkedList<int> list;
    fillWith(list, 1, 3);
    CHECK(list.contains(3));
    CHECK(!list.contains(4));
}

TEST_CASE(circularCopyKeepsCursorPositionAndIsDeep)
{
    CircularLinkedList<int> original;
    fillWith(original, 1, 4);
    original.advance();
    original.advance(); // cursor on 3

    CircularLinkedList<int> copy(original);
    CHECK_EQ(copy.current(), 3);
    copy.advance();
    CHECK_EQ(copy.current(), 4);
    CHECK_EQ(original.current(), 3); // independent cursors

    copy.popFront();
    const int expected[] = {1, 2, 3, 4};
    CHECK(contentsAre(original, expected, 4));

    CircularLinkedList<int> assigned;
    assigned.pushBack(100);
    assigned = original;
    CHECK(contentsAre(assigned, expected, 4));
    CHECK_EQ(assigned.current(), 3);

    CircularLinkedList<int>& alias = original;
    original = alias;
    CHECK(contentsAre(original, expected, 4));
    CHECK_EQ(original.current(), 3);
}

TEST_CASE(circularFreesEveryNode)
{
    Tracked::alive = 0;
    {
        CircularLinkedList<Tracked> list;
        for (int i = 0; i < 6; ++i)
            list.pushBack(Tracked(i));
        list.popFront();
        list.removeFirst(Tracked(3));
        CHECK_EQ(Tracked::alive, 4);

        CircularLinkedList<Tracked> copy(list);
        CHECK_EQ(Tracked::alive, 8);
        copy.clear();
        CHECK_EQ(Tracked::alive, 4);
    }
    CHECK_EQ(Tracked::alive, 0);
}

int main()
{
    return tf::runAll("Linked lists");
}
