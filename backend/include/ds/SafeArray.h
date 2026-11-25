#ifndef WORDLE_DSA_SAFE_ARRAY_H
#define WORDLE_DSA_SAFE_ARRAY_H

// SafeArray<T>: a bounds-checked, growable 1D dynamic array.
//
// Used for: storing the dictionary and other lists in place of vector.
//
// Every index is checked. An out-of-range access throws an error
// instead of reading or writing memory that does not belong to the array.
//
// Rule of Three: the array owns a heap buffer, so it defines a destructor,
// a deep-copying copy constructor and a deep-copying copy assignment operator.
//
// Requirement: T must be default-constructible and copy-assignable.
//
// Complexity:
//   operator[] / at / front / back ....... O(1)
//   pushBack ............................. O(1) amortised (capacity doubles)
//   popBack .............................. O(1)
//   insertAt / removeAt .................. O(n) (elements are shifted)
//   resize / reserve / copy .............. O(n)

#include <cstddef>
#include <string>

using namespace std;

template <typename T>
class SafeArray
{
public:
    // ----- construction / Rule of Three -----

    SafeArray(): data_(nullptr), size_(0), capacity_(0){}

    explicit SafeArray(size_t size, const T& fill = T())
        : data_(nullptr), size_(0), capacity_(0)
    { 
        if (size > 0)
        {
            data_ = new T[size];
            capacity_ = size;
            for (size_t i = 0; i < size; ++i)
                data_[i] = fill;
            size_ = size;
        }
    }

    ~SafeArray()
    {
        delete[] data_;
    }

    SafeArray(const SafeArray& other)
        : data_(nullptr), size_(0), capacity_(0)
    {
        if (other.size_ > 0)
        {
            data_ = new T[other.size_];
            for (size_t i = 0; i < other.size_; ++i)
                data_[i] = other.data_[i];
            size_ = other.size_;
            capacity_ = other.size_;
        }
    }

    SafeArray& operator=(const SafeArray& other)
    {
        if (this == &other)
            return *this;

        // Build the copy first so *this is unchanged if allocation throws.
        T* newData = nullptr;
        if (other.size_ > 0)
        {
            newData = new T[other.size_];
            for (size_t i = 0; i < other.size_; ++i)
                newData[i] = other.data_[i];
        }

        delete[] data_;
        data_ = newData;
        size_ = other.size_;
        capacity_ = other.size_;
        return *this;
    }

    // ----- element access (always bounds-checked) -----

    T& operator[](size_t index)
    {
        checkIndex(index);
        return data_[index];
    }

    const T& operator[](size_t index) const
    {
        checkIndex(index);
        return data_[index];
    }

    T& at(size_t index) { return (*this)[index]; }
    const T& at(size_t index) const { return (*this)[index]; }

    T& front()
    {
        checkNotEmpty();
        return data_[0];
    }

    const T& front() const
    {
        checkNotEmpty();
        return data_[0];
    }

    T& back()
    {
        checkNotEmpty();
        return data_[size_ - 1];
    }

    const T& back() const
    {
        checkNotEmpty();
        return data_[size_ - 1];
    }

    // ----- modification -----

    // GUIDE 1.2: capacity doubles when full -> O(1) amortised push
    void pushBack(const T& value)
    {
        if (size_ == capacity_)
            reserve(capacity_ == 0 ? kInitialCapacity : capacity_ * 2);
        data_[size_] = value;
        ++size_;
    }

    void popBack()
    {
        checkNotEmpty();
        --size_;
        data_[size_] = T(); // release whatever the old element held (e.g. a long string)
    }

    // Inserts value so that it ends up at position index.
    // index == size() is allowed and appends.
    void insertAt(size_t index, const T& value)
    {
        if (index > size_)
            throw "SafeArray: index out of range";

        if (size_ == capacity_)
            reserve(capacity_ == 0 ? kInitialCapacity : capacity_ * 2);

        for (size_t i = size_; i > index; --i)
            data_[i] = data_[i - 1];
        data_[index] = value;
        ++size_;
    }

    void removeAt(size_t index)
    {
        checkIndex(index);
        for (size_t i = index; i + 1 < size_; ++i)
            data_[i] = data_[i + 1];
        --size_;
        data_[size_] = T();
    }

    // Changes the number of elements. Existing elements are kept,
    // new elements are set to fill, extra elements are dropped.
    void resize(size_t newSize, const T& fill = T())
    {
        if (newSize > capacity_)
            reserve(newSize);

        for (size_t i = size_; i < newSize; ++i)
            data_[i] = fill;
        for (size_t i = newSize; i < size_; ++i)
            data_[i] = T();

        size_ = newSize;
    }

    // Makes sure the array can hold at least newCapacity elements
    // without reallocating. Never shrinks.
    void reserve(size_t newCapacity)
    {
        if (newCapacity <= capacity_)
            return;

        T* newData = new T[newCapacity];
        for (size_t i = 0; i < size_; ++i)
            newData[i] = data_[i];

        delete[] data_;
        data_ = newData;
        capacity_ = newCapacity;
    }

    void swapAt(size_t i, size_t j)
    {
        checkIndex(i);
        checkIndex(j);
        if (i == j)
            return;
        T temp = data_[i];
        data_[i] = data_[j];
        data_[j] = temp;
    }

    // Removes every element and frees the buffer.
    void clear()
    {
        delete[] data_;
        data_ = nullptr;
        size_ = 0;
        capacity_ = 0;
    }

    // ----- queries -----

    size_t size() const { return size_; }
    size_t capacity() const { return capacity_; }
    bool empty() const { return size_ == 0; }

    // ----- iteration (for range-based for loops; stays inside [0, size)) -----

    T* begin() { return data_; }
    T* end() { return data_ + size_; }
    const T* begin() const { return data_; }
    const T* end() const { return data_ + size_; }

private:
    static constexpr size_t kInitialCapacity = 4;

    T* data_;
    size_t size_;
    size_t capacity_;

    // GUIDE 1.2: the "safe" part - every [] goes through this check
    void checkIndex(size_t index) const
    {
        if (index >= size_)
            throw "SafeArray: index out of range";
    }

    void checkNotEmpty() const
    {
        if (size_ == 0)
            throw "SafeArray: the array is empty";
    }
};

#endif // WORDLE_DSA_SAFE_ARRAY_H
