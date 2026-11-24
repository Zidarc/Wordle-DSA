#ifndef WORDLE_DSA_ARRAY_QUEUE_H
#define WORDLE_DSA_ARRAY_QUEUE_H

// ArrayQueue<T>: first in, first out, stored in a circular array.
//
// This is the second implementation strategy for a queue (the first is
// Queue<T>, built on a singly linked list). A plain array queue would have to
// shift every element left on each dequeue (O(n)). A circular array avoids
// that: front_ and the back position simply wrap around with % capacity_.
//
//     capacity 6, size 4, front_ = 4
//
//       index:  0    1    2    3    4    5
//             [ C ][ D ][   ][   ][ A ][ B ]
//                        ^back      ^front
//
//     order of removal: A, B, C, D
//
// Two modes:
//   - growing (default): when full, the array doubles in size and the
//     elements are copied out in queue order, so the wrap-around is undone.
//   - bounded ring buffer (makeBounded = true): when full, enqueue drops the
//     oldest element. Used for the server's "last 10 games" buffer.
//
// Rule of Three: the queue owns data_, so it defines a destructor, a deep-
// copying copy constructor and a deep-copying copy assignment operator.
//
// dequeue / front / back on an empty queue throw std::out_of_range.
//
// Complexity: enqueue O(1) amortised, dequeue / front / back / at O(1).

#include <cstddef>
#include <stdexcept>
#include <string>

template <typename T>
class ArrayQueue
{
public:
    static constexpr std::size_t kDefaultCapacity = 8;

    explicit ArrayQueue(std::size_t capacity = kDefaultCapacity, bool makeBounded = false)
        : data_(nullptr),
          front_(0),
          size_(0),
          capacity_(capacity == 0 ? 1 : capacity),
          bounded_(makeBounded)
    {
        data_ = new T[capacity_];
    }

    ~ArrayQueue()
    {
        delete[] data_;
    }

    ArrayQueue(const ArrayQueue& other)
        : data_(nullptr), front_(0), size_(0), capacity_(other.capacity_), bounded_(other.bounded_)
    {
        data_ = new T[capacity_];
        copyElementsFrom(other);
    }

    ArrayQueue& operator=(const ArrayQueue& other)
    {
        if (this == &other)
            return *this;

        ArrayQueue temp(other); // copy first: *this is unchanged if it throws
        delete[] data_;
        data_ = temp.data_;
        front_ = temp.front_;
        size_ = temp.size_;
        capacity_ = temp.capacity_;
        bounded_ = temp.bounded_;
        temp.data_ = nullptr; // temp's destructor now deletes nothing
        return *this;
    }

    // Adds value at the back. In bounded mode a full queue first drops its
    // oldest element; returns true when that happened.
    // GUIDE 1.4: (front_ + size_) % capacity_ is the wrap-around
    bool enqueue(const T& value)
    {
        bool dropped = false;
        if (size_ == capacity_)
        {
            if (bounded_)
            {
                front_ = (front_ + 1) % capacity_;
                --size_;
                dropped = true;
            }
            else
            {
                grow();
            }
        }
        data_[(front_ + size_) % capacity_] = value;
        ++size_;
        return dropped;
    }

    // Removes the front element and returns it.
    T dequeue()
    {
        checkNotEmpty("dequeue");
        T value = data_[front_];
        front_ = (front_ + 1) % capacity_;
        --size_;
        return value;
    }

    const T& front() const
    {
        checkNotEmpty("front");
        return data_[front_];
    }

    const T& back() const
    {
        checkNotEmpty("back");
        return data_[(front_ + size_ - 1) % capacity_];
    }

    // Element i places behind the front (0 = front). Throws std::out_of_range.
    const T& at(std::size_t i) const
    {
        if (i >= size_)
            throw std::out_of_range("ArrayQueue::at: index " + std::to_string(i) +
                                    " out of range (size " + std::to_string(size_) + ")");
        return data_[(front_ + i) % capacity_];
    }

    void clear()
    {
        front_ = 0;
        size_ = 0;
    }

    std::size_t size() const { return size_; }
    std::size_t capacity() const { return capacity_; }
    bool empty() const { return size_ == 0; }
    bool full() const { return size_ == capacity_; }
    bool bounded() const { return bounded_; }

    // Raw array slot of the front element; lets the tests see the wrap-around.
    std::size_t frontIndex() const { return front_; }

private:
    T* data_;
    std::size_t front_;
    std::size_t size_;
    std::size_t capacity_;
    bool bounded_;

    void grow()
    {
        std::size_t newCapacity = capacity_ * 2;
        T* bigger = new T[newCapacity];
        for (std::size_t i = 0; i < size_; ++i)
            bigger[i] = data_[(front_ + i) % capacity_];
        delete[] data_;
        data_ = bigger;
        front_ = 0;
        capacity_ = newCapacity;
    }

    // Copies other's elements in queue order, starting at slot 0.
    void copyElementsFrom(const ArrayQueue& other)
    {
        for (std::size_t i = 0; i < other.size_; ++i)
            data_[i] = other.data_[(other.front_ + i) % other.capacity_];
        front_ = 0;
        size_ = other.size_;
    }

    void checkNotEmpty(const char* operation) const
    {
        if (size_ == 0)
            throw std::out_of_range(std::string("ArrayQueue::") + operation + " on an empty queue");
    }
};

#endif // WORDLE_DSA_ARRAY_QUEUE_H
