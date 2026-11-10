#ifndef WORDLE_DSA_QUEUE_H
#define WORDLE_DSA_QUEUE_H

// Queue<T>: first in, first out, built on SinglyLinkedList<T>.
//
// Used for: hint order (Day 8). At the start of a game the five letter
// positions are shuffled into the queue, and each hint dequeues the next one.
//
// Elements join at the tail (pushBack, O(1) thanks to the tail pointer) and
// leave from the head (popFront, O(1)).
//
// Rule of Three: Queue owns no raw memory itself. Its only member is a
// SinglyLinkedList, which already deep-copies and frees its nodes, so the
// compiler-generated destructor, copy constructor and copy assignment are
// correct. They are declared explicitly below to show this is deliberate.
//
// dequeue / front / back on an empty queue throw std::out_of_range.
//
// Complexity: enqueue, dequeue, front, back, size, empty are all O(1).

#include "ds/SinglyLinkedList.h"

#include <cstddef>
#include <stdexcept>
#include <string>

template <typename T>
class Queue
{
public:
    using ConstIterator = typename SinglyLinkedList<T>::ConstIterator;

    Queue() = default;
    ~Queue() = default;
    Queue(const Queue& other) = default;            // deep copy via SinglyLinkedList
    Queue& operator=(const Queue& other) = default; // deep copy via SinglyLinkedList

    void enqueue(const T& value)
    {
        list_.pushBack(value);
    }

    // Removes the front element and returns it.
    T dequeue()
    {
        checkNotEmpty("dequeue");
        T value = list_.front();
        list_.popFront();
        return value;
    }

    T& front()
    {
        checkNotEmpty("front");
        return list_.front();
    }

    const T& front() const
    {
        checkNotEmpty("front");
        return list_.front();
    }

    T& back()
    {
        checkNotEmpty("back");
        return list_.back();
    }

    const T& back() const
    {
        checkNotEmpty("back");
        return list_.back();
    }

    void clear() { list_.clear(); }
    std::size_t size() const { return list_.size(); }
    bool empty() const { return list_.empty(); }

    // Iterates from the front of the queue to the back, without removing anything.
    ConstIterator begin() const { return list_.begin(); }
    ConstIterator end() const { return list_.end(); }

private:
    SinglyLinkedList<T> list_;

    void checkNotEmpty(const char* operation) const
    {
        if (list_.empty())
            throw std::out_of_range(std::string("Queue::") + operation + " on an empty queue");
    }
};

#endif // WORDLE_DSA_QUEUE_H
