#ifndef WORDLE_DSA_ARRAY_STACK_H
#define WORDLE_DSA_ARRAY_STACK_H

// ArrayStack<T>: last in, first out, stored in a dynamic array.
//
// This is the second implementation strategy for a stack (the first is
// Stack<T>, built on a singly linked list). Both have the same interface, so
// the two strategies can be compared directly:
//
//                  Stack<T> (linked)          ArrayStack<T> (array)
//   push           O(1), one new node         O(1) amortised, may grow the array
//   pop            O(1), one delete           O(1), no memory released
//   memory         one pointer per element    unused capacity at the end
//   locality       nodes spread over heap     elements next to each other
//
// Used for: the explicit stack in iterative quick sort and in the solver's
// iterative backtracking, where many small push/pop pairs happen in a row.
//
// The top of the stack is data_[size_ - 1]. When the array is full its
// capacity is doubled, so a run of n pushes costs O(n) in total.
//
// Rule of Three: the stack owns data_, so it defines a destructor, a deep-
// copying copy constructor and a deep-copying copy assignment operator.
//
// pop / top on an empty stack throw std::out_of_range.

#include <cstddef>
#include <stdexcept>
#include <string>

template <typename T>
class ArrayStack
{
public:
    static constexpr std::size_t kDefaultCapacity = 8;

    explicit ArrayStack(std::size_t initialCapacity = kDefaultCapacity)
        : data_(nullptr), size_(0), capacity_(initialCapacity == 0 ? 1 : initialCapacity)
    {
        data_ = new T[capacity_];
    }

    ~ArrayStack()
    {
        delete[] data_;
    }

    ArrayStack(const ArrayStack& other)
        : data_(nullptr), size_(other.size_), capacity_(other.capacity_)
    {
        data_ = new T[capacity_];
        for (std::size_t i = 0; i < size_; ++i)
            data_[i] = other.data_[i];
    }

    ArrayStack& operator=(const ArrayStack& other)
    {
        if (this == &other)
            return *this;

        // Allocate and copy first, so *this is unchanged if new throws.
        T* copy = new T[other.capacity_];
        for (std::size_t i = 0; i < other.size_; ++i)
            copy[i] = other.data_[i];

        delete[] data_;
        data_ = copy;
        size_ = other.size_;
        capacity_ = other.capacity_;
        return *this;
    }

    void push(const T& value)
    {
        if (size_ == capacity_)
            grow();
        data_[size_] = value;
        ++size_;
    }

    // Removes the top element and returns it.
    T pop()
    {
        checkNotEmpty("pop");
        --size_;
        return data_[size_];
    }

    T& top()
    {
        checkNotEmpty("top");
        return data_[size_ - 1];
    }

    const T& top() const
    {
        checkNotEmpty("top");
        return data_[size_ - 1];
    }

    const T& peek() const { return top(); }

    void clear() { size_ = 0; }
    std::size_t size() const { return size_; }
    std::size_t capacity() const { return capacity_; }
    bool empty() const { return size_ == 0; }

private:
    T* data_;
    std::size_t size_;
    std::size_t capacity_;

    void grow()
    {
        std::size_t newCapacity = capacity_ * 2;
        T* bigger = new T[newCapacity];
        for (std::size_t i = 0; i < size_; ++i)
            bigger[i] = data_[i];
        delete[] data_;
        data_ = bigger;
        capacity_ = newCapacity;
    }

    void checkNotEmpty(const char* operation) const
    {
        if (size_ == 0)
            throw std::out_of_range(std::string("ArrayStack::") + operation + " on an empty stack");
    }
};

#endif // WORDLE_DSA_ARRAY_STACK_H
