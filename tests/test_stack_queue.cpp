#include "TestFramework.h"
#include "ds/Queue.h"
#include "ds/Stack.h"

#include <stdexcept>
#include <string>

// ===========================================================================
// Stack
// ===========================================================================

TEST_CASE(stackStartsEmpty)
{
    Stack<int> stack;
    CHECK(stack.empty());
    CHECK_EQ(stack.size(), 0u);
    CHECK(stack.begin() == stack.end());
}

TEST_CASE(stackPopAndPeekOnEmptyThrow)
{
    Stack<int> stack;
    CHECK_THROWS(stack.pop(), std::out_of_range);
    CHECK_THROWS(stack.top(), std::out_of_range);
    CHECK_THROWS(stack.peek(), std::out_of_range);
}

TEST_CASE(stackIsLastInFirstOut)
{
    Stack<int> stack;
    for (int i = 1; i <= 5; ++i)
        stack.push(i);

    CHECK_EQ(stack.size(), 5u);
    for (int expected = 5; expected >= 1; --expected)
        CHECK_EQ(stack.pop(), expected);
    CHECK(stack.empty());
}

TEST_CASE(stackPeekDoesNotRemove)
{
    Stack<std::string> stack;
    stack.push("CRANE");
    stack.push("SLATE");
    CHECK_EQ(stack.peek(), std::string("SLATE"));
    CHECK_EQ(stack.top(), std::string("SLATE"));
    CHECK_EQ(stack.size(), 2u);

    stack.top() = "PLUMB"; // top() gives write access
    CHECK_EQ(stack.pop(), std::string("PLUMB"));
    CHECK_EQ(stack.pop(), std::string("CRANE"));
}

TEST_CASE(stackIteratesMostRecentFirst)
{
    // Guess history panel: newest guess first, nothing removed.
    Stack<std::string> history;
    history.push("CRANE");
    history.push("SLOTH");
    history.push("PLUMB");

    const char* expected[] = {"PLUMB", "SLOTH", "CRANE"};
    int i = 0;
    for (const std::string& guess : history)
    {
        CHECK_EQ(guess, std::string(expected[i]));
        ++i;
    }
    CHECK_EQ(i, 3);
    CHECK_EQ(history.size(), 3u);
}

TEST_CASE(stackPushAfterEmptying)
{
    Stack<int> stack;
    stack.push(1);
    stack.pop();
    stack.push(2);
    CHECK_EQ(stack.top(), 2);
    stack.clear();
    CHECK(stack.empty());
}

TEST_CASE(stackCopyIsDeep)
{
    Stack<int> original;
    original.push(1);
    original.push(2);

    Stack<int> copy(original);
    copy.pop();
    copy.push(99);

    CHECK_EQ(original.size(), 2u);
    CHECK_EQ(original.top(), 2);
    CHECK_EQ(copy.top(), 99);

    Stack<int> assigned;
    assigned.push(7);
    assigned = original;
    CHECK_EQ(assigned.pop(), 2);
    CHECK_EQ(assigned.pop(), 1);
    CHECK_EQ(original.size(), 2u);

    Stack<int>& alias = original;
    original = alias;
    CHECK_EQ(original.size(), 2u);
    CHECK_EQ(original.top(), 2);
}

TEST_CASE(stackHandlesManyElements)
{
    Stack<int> stack;
    for (int i = 0; i < 10000; ++i)
        stack.push(i);
    for (int i = 9999; i >= 0; --i)
        CHECK_EQ(stack.pop(), i);
    CHECK(stack.empty());
}

// ===========================================================================
// Queue
// ===========================================================================

TEST_CASE(queueStartsEmpty)
{
    Queue<int> queue;
    CHECK(queue.empty());
    CHECK_EQ(queue.size(), 0u);
    CHECK(queue.begin() == queue.end());
}

TEST_CASE(queueDequeueAndFrontOnEmptyThrow)
{
    Queue<int> queue;
    CHECK_THROWS(queue.dequeue(), std::out_of_range);
    CHECK_THROWS(queue.front(), std::out_of_range);
    CHECK_THROWS(queue.back(), std::out_of_range);
}

TEST_CASE(queueIsFirstInFirstOut)
{
    Queue<int> queue;
    for (int i = 1; i <= 5; ++i)
        queue.enqueue(i);

    CHECK_EQ(queue.front(), 1);
    CHECK_EQ(queue.back(), 5);
    for (int expected = 1; expected <= 5; ++expected)
        CHECK_EQ(queue.dequeue(), expected);
    CHECK(queue.empty());
}

TEST_CASE(queueInterleavedEnqueueDequeue)
{
    Queue<int> queue;
    queue.enqueue(1);
    queue.enqueue(2);
    CHECK_EQ(queue.dequeue(), 1);
    queue.enqueue(3);
    CHECK_EQ(queue.dequeue(), 2);
    CHECK_EQ(queue.dequeue(), 3);
    CHECK(queue.empty());

    queue.enqueue(4); // works again after being emptied (tail was reset)
    CHECK_EQ(queue.front(), 4);
    CHECK_EQ(queue.back(), 4);
}

TEST_CASE(queueHintPositionsComeOutInOrder)
{
    // Hint system: shuffled positions go in, each hint takes the next one.
    Queue<int> hints;
    const int shuffled[] = {3, 0, 4, 1, 2};
    for (int position : shuffled)
        hints.enqueue(position);

    for (int position : shuffled)
        CHECK_EQ(hints.dequeue(), position);
}

TEST_CASE(queueIteratesFrontToBack)
{
    Queue<std::string> queue;
    queue.enqueue("A");
    queue.enqueue("B");
    queue.enqueue("C");

    std::string joined;
    for (const std::string& s : queue)
        joined += s;
    CHECK_EQ(joined, std::string("ABC"));
    CHECK_EQ(queue.size(), 3u);
}

TEST_CASE(queueCopyIsDeep)
{
    Queue<int> original;
    original.enqueue(1);
    original.enqueue(2);

    Queue<int> copy(original);
    copy.dequeue();
    copy.enqueue(3);
    CHECK_EQ(original.front(), 1);
    CHECK_EQ(original.size(), 2u);
    CHECK_EQ(copy.front(), 2);
    CHECK_EQ(copy.back(), 3);

    Queue<int> assigned;
    assigned.enqueue(9);
    assigned = original;
    CHECK_EQ(assigned.dequeue(), 1);
    CHECK_EQ(assigned.dequeue(), 2);
    CHECK_EQ(original.size(), 2u);

    Queue<int>& alias = original;
    original = alias;
    CHECK_EQ(original.front(), 1);
    CHECK_EQ(original.back(), 2);
}

TEST_CASE(queueClear)
{
    Queue<int> queue;
    queue.enqueue(1);
    queue.enqueue(2);
    queue.clear();
    CHECK(queue.empty());
    CHECK_THROWS(queue.front(), std::out_of_range);
}

int main()
{
    return tf::runAll("Stack and Queue");
}
