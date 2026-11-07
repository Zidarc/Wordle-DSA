#ifndef WORDLE_DSA_STACK_H
#define WORDLE_DSA_STACK_H

// Stack<T>: last in, first out, built on SinglyLinkedList<T>.
//
// Used for:
//   - guess history (Day 8): the most recent guess is on top
//   - simulating recursion (Day 3): iterative in-order traversal of the AVL tree
//
// The top of the stack is the head of the list, so push and pop are both
// O(1) pushFront / popFront. (Using the tail would make pop O(n), because a
// singly linked list cannot step back from its tail.)
//
// Rule of Three: Stack owns no raw memory itself. Its only member is a
// SinglyLinkedList, which already deep-copies and frees its nodes, so the
// compiler-generated destructor, copy constructor and copy assignment are
// correct. They are declared explicitly below to show this is deliberate.
//
// pop / top on an empty stack throw std::out_of_range.
//
// Complexity: push, pop, top, size, empty are all O(1).

#include "ds/SinglyLinkedList.h"

#include <cstddef>
#include <stdexcept>
#include <string>

template <typename T>
class Stack
{
public:
    using ConstIterator = typename SinglyLinkedList<T>::ConstIterator;

    Stack() = default;
    ~Stack() = default;
    Stack(const Stack& other) = default;            // deep copy via SinglyLinkedList
    Stack& operator=(const Stack& other) = default; // deep copy via SinglyLinkedList

    void push(const T& value)
    {
        list_.pushFront(value);
    }

    // Removes the top element and returns it.
    T pop()
    {
        checkNotEmpty("pop");
        T value = list_.front();
        list_.popFront();
        return value;
    }

    T& top()
    {
        checkNotEmpty("top");
        return list_.front();
    }

    const T& top() const
    {
        checkNotEmpty("top");
        return list_.front();
    }

    // Same as top(); both names are common.
    const T& peek() const { return top(); }

    void clear() { list_.clear(); }
    std::size_t size() const { return list_.size(); }
    bool empty() const { return list_.empty(); }

    // Iterates from the top of the stack to the bottom (most recent guess first),
    // without removing anything.
    ConstIterator begin() const { return list_.begin(); }
    ConstIterator end() const { return list_.end(); }

private:
    SinglyLinkedList<T> list_;

    void checkNotEmpty(const char* operation) const
    {
        if (list_.empty())
            throw std::out_of_range(std::string("Stack::") + operation + " on an empty stack");
    }
};

#endif // WORDLE_DSA_STACK_H
