#ifndef WORDLE_DSA_SAFE_ARRAY_H
#define WORDLE_DSA_SAFE_ARRAY_H

// SafeArray<T>: a bounds-checked, growable 1D dynamic array.
//
// Used for: storing the dictionary and other lists in place of std::vector.
//
// Every index is checked. An out-of-range access throws std::out_of_range
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
#include <stdexcept>
#include <string>

template <typename T>
class SafeArray
{
public:
    // ----- construction / Rule of Three -----

    SafeArray()
        : data_(nullptr), size_(0), capacity_(0)
    {
    }

    explicit SafeArray(std::size_t size, const T& fill = T())
        : data_(nullptr), size_(0), capacity_(0)
    {
        if (size > 0)
        {
            data_ = new T[size];
            capacity_ = size;
            for (std::size_t i = 0; i < size; ++i)
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
            for (std::size_t i = 0; i < other.size_; ++i)
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
            for (std::size_t i = 0; i < other.size_; ++i)
                newData[i] = other.data_[i];
        }

        delete[] data_;
        data_ = newData;
        size_ = other.size_;
        capacity_ = other.size_;
        return *this;
    }

    // ----- element access (always bounds-checked) -----

    T& operator[](std::size_t index)
    {
        checkIndex(index);
        return data_[index];
    }

    const T& operator[](std::size_t index) const
    {
        checkIndex(index);
        return data_[index];
    }

    T& at(std::size_t index) { return (*this)[index]; }
    const T& at(std::size_t index) const { return (*this)[index]; }

    T& front()
    {
        checkNotEmpty("front");
        return data_[0];
    }

    const T& front() const
    {
        checkNotEmpty("front");
        return data_[0];
    }

    T& back()
    {
        checkNotEmpty("back");
        return data_[size_ - 1];
    }

    const T& back() const
    {
        checkNotEmpty("back");
        return data_[size_ - 1];
    }

    // ----- modification -----

    void pushBack(const T& value)
    {
        if (size_ == capacity_)
            reserve(capacity_ == 0 ? kInitialCapacity : capacity_ * 2);
        data_[size_] = value;
        ++size_;
    }

    void popBack()
    {
        checkNotEmpty("popBack");
        --size_;
        data_[size_] = T(); // release whatever the old element held (e.g. a long string)
    }

    // Inserts value so that it ends up at position index.
    // index == size() is allowed and appends.
    void insertAt(std::size_t index, const T& value)
    {
        if (index > size_)
            throw std::out_of_range(rangeMessage("insertAt", index, size_ + 1));

        if (size_ == capacity_)
            reserve(capacity_ == 0 ? kInitialCapacity : capacity_ * 2);

        for (std::size_t i = size_; i > index; --i)
            data_[i] = data_[i - 1];
        data_[index] = value;
        ++size_;
    }

    void removeAt(std::size_t index)
    {
        checkIndex(index);
        for (std::size_t i = index; i + 1 < size_; ++i)
            data_[i] = data_[i + 1];
        --size_;
        data_[size_] = T();
    }

    // Changes the number of elements. Existing elements are kept,
    // new elements are set to fill, extra elements are dropped.
    void resize(std::size_t newSize, const T& fill = T())
    {
        if (newSize > capacity_)
            reserve(newSize);

        for (std::size_t i = size_; i < newSize; ++i)
            data_[i] = fill;
        for (std::size_t i = newSize; i < size_; ++i)
            data_[i] = T();

        size_ = newSize;
    }

    // Makes sure the array can hold at least newCapacity elements
    // without reallocating. Never shrinks.
    void reserve(std::size_t newCapacity)
    {
        if (newCapacity <= capacity_)
            return;

        T* newData = new T[newCapacity];
        for (std::size_t i = 0; i < size_; ++i)
            newData[i] = data_[i];

        delete[] data_;
        data_ = newData;
        capacity_ = newCapacity;
    }

    void swapAt(std::size_t i, std::size_t j)
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

    std::size_t size() const { return size_; }
    std::size_t capacity() const { return capacity_; }
    bool empty() const { return size_ == 0; }

    // ----- iteration (for range-based for loops; stays inside [0, size)) -----

    T* begin() { return data_; }
    T* end() { return data_ + size_; }
    const T* begin() const { return data_; }
    const T* end() const { return data_ + size_; }

private:
    static constexpr std::size_t kInitialCapacity = 4;

    T* data_;
    std::size_t size_;
    std::size_t capacity_;

    void checkIndex(std::size_t index) const
    {
        if (index >= size_)
            throw std::out_of_range(rangeMessage("index", index, size_));
    }

    void checkNotEmpty(const char* operation) const
    {
        if (size_ == 0)
            throw std::out_of_range(std::string("SafeArray::") + operation + " on an empty array");
    }

    static std::string rangeMessage(const char* operation, std::size_t index, std::size_t limit)
    {
        return std::string("SafeArray::") + operation + ": index " + std::to_string(index) +
               " is out of range (valid: 0 to " +
               (limit == 0 ? std::string("none") : std::to_string(limit - 1)) + ")";
    }
};

#endif // WORDLE_DSA_SAFE_ARRAY_H
