// Tests for the array-based stack and the circular-array queue.

#include "TestFramework.h"

#include "ds/ArrayQueue.h"
#include "ds/ArrayStack.h"

#include <stdexcept>
#include <string>

// ---------------------------------------------------------------------------
// ArrayStack
// ---------------------------------------------------------------------------

TEST_CASE(arrayStackStartsEmpty)
{
    ArrayStack<int> s;
    CHECK(s.empty());
    CHECK_EQ(s.size(), 0u);
    CHECK_THROWS(s.pop());
    CHECK_THROWS(s.top());
}

TEST_CASE(arrayStackIsLastInFirstOut)
{
    ArrayStack<std::string> s;
    s.push("CRANE");
    s.push("SLATE");
    s.push("BRAVE");
    CHECK_EQ(s.top(), std::string("BRAVE"));
    CHECK_EQ(s.pop(), std::string("BRAVE"));
    CHECK_EQ(s.pop(), std::string("SLATE"));
    CHECK_EQ(s.pop(), std::string("CRANE"));
    CHECK(s.empty());
}

TEST_CASE(arrayStackGrowsByDoubling)
{
    ArrayStack<int> s(2);
    CHECK_EQ(s.capacity(), 2u);
    for (int i = 0; i < 5; ++i)
        s.push(i);
    CHECK_EQ(s.size(), 5u);
    CHECK_EQ(s.capacity(), 8u); // 2 -> 4 -> 8
    for (int i = 4; i >= 0; --i)
        CHECK_EQ(s.pop(), i);
}

TEST_CASE(arrayStackZeroCapacityStillWorks)
{
    ArrayStack<int> s(0);
    s.push(7);
    CHECK_EQ(s.top(), 7);
}

TEST_CASE(arrayStackCopyIsDeep)
{
    ArrayStack<int> a;
    a.push(1);
    a.push(2);

    ArrayStack<int> b(a);
    b.push(3);
    CHECK_EQ(a.size(), 2u);
    CHECK_EQ(b.size(), 3u);

    ArrayStack<int> c;
    c = a;
    c.pop();
    CHECK_EQ(a.top(), 2);
    CHECK_EQ(c.top(), 1);

    c = c; // self-assignment
    CHECK_EQ(c.top(), 1);
}

TEST_CASE(arrayStackClear)
{
    ArrayStack<int> s;
    s.push(1);
    s.push(2);
    s.clear();
    CHECK(s.empty());
    s.push(9);
    CHECK_EQ(s.top(), 9);
}

// ---------------------------------------------------------------------------
// ArrayQueue
// ---------------------------------------------------------------------------

TEST_CASE(arrayQueueStartsEmpty)
{
    ArrayQueue<int> q;
    CHECK(q.empty());
    CHECK_THROWS(q.dequeue());
    CHECK_THROWS(q.front());
    CHECK_THROWS(q.back());
    CHECK_THROWS(q.at(0));
}

TEST_CASE(arrayQueueIsFirstInFirstOut)
{
    ArrayQueue<int> q;
    for (int i = 1; i <= 4; ++i)
        q.enqueue(i);
    CHECK_EQ(q.front(), 1);
    CHECK_EQ(q.back(), 4);
    for (int i = 1; i <= 4; ++i)
        CHECK_EQ(q.dequeue(), i);
    CHECK(q.empty());
}

TEST_CASE(arrayQueueWrapsAroundWithoutGrowing)
{
    ArrayQueue<int> q(4);
    q.enqueue(1);
    q.enqueue(2);
    q.enqueue(3);
    CHECK_EQ(q.dequeue(), 1);
    CHECK_EQ(q.dequeue(), 2);
    q.enqueue(4);
    q.enqueue(5); // goes into slot 0
    q.enqueue(6); // goes into slot 1
    CHECK_EQ(q.capacity(), 4u);
    CHECK(q.full());
    CHECK_EQ(q.frontIndex(), 2u);
    CHECK_EQ(q.at(0), 3);
    CHECK_EQ(q.at(3), 6);
    for (int i = 3; i <= 6; ++i)
        CHECK_EQ(q.dequeue(), i);
}

TEST_CASE(arrayQueueGrowsAndKeepsOrderAfterWrap)
{
    ArrayQueue<int> q(3);
    q.enqueue(1);
    q.enqueue(2);
    q.enqueue(3);
    q.dequeue();
    q.enqueue(4); // wrapped
    q.enqueue(5); // full -> grow
    CHECK_EQ(q.capacity(), 6u);
    CHECK_EQ(q.frontIndex(), 0u);
    for (int i = 2; i <= 5; ++i)
        CHECK_EQ(q.dequeue(), i);
}

TEST_CASE(boundedQueueDropsOldest)
{
    ArrayQueue<std::string> recent(3, true);
    CHECK(!recent.enqueue("A"));
    CHECK(!recent.enqueue("B"));
    CHECK(!recent.enqueue("C"));
    CHECK(recent.enqueue("D")); // A is dropped
    CHECK(recent.enqueue("E")); // B is dropped
    CHECK_EQ(recent.size(), 3u);
    CHECK_EQ(recent.capacity(), 3u);
    CHECK_EQ(recent.front(), std::string("C"));
    CHECK_EQ(recent.back(), std::string("E"));
}

TEST_CASE(arrayQueueCopyIsDeep)
{
    ArrayQueue<int> a(3);
    a.enqueue(1);
    a.enqueue(2);
    a.dequeue();
    a.enqueue(3);
    a.enqueue(4); // a is wrapped: front slot 1

    ArrayQueue<int> b(a);
    CHECK_EQ(b.size(), 3u);
    CHECK_EQ(b.frontIndex(), 0u); // the copy is laid out from slot 0
    b.dequeue();
    CHECK_EQ(a.front(), 2);
    CHECK_EQ(b.front(), 3);

    ArrayQueue<int> c;
    c = a;
    CHECK_EQ(c.at(2), 4);
    c = c;
    CHECK_EQ(c.at(0), 2);
}

int main()
{
    return tf::runAll("ArrayStack / ArrayQueue");
}
