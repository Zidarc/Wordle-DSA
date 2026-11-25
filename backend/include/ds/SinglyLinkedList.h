#ifndef WORDLE_DSA_SINGLY_LINKED_LIST_H
#define WORDLE_DSA_SINGLY_LINKED_LIST_H

// SinglyLinkedList<T>: nodes linked in one direction, with head and tail pointers.
//
// Used for:
//   - the base of Stack<T> and Queue<T>
//   - the collision chains of the hash table (Day 4), through findIf / removeIf
//
// Keeping a tail pointer makes pushBack O(1), which Queue<T> relies on.
//
// Rule of Three: the list owns its nodes, so it defines a destructor, a deep-copying
// copy constructor and a deep-copying copy assignment operator.
//
// Operations on an empty list (front, back, popFront, popBack) and invalid
// indexes throw an error.
//
// Complexity:
//   pushFront / pushBack / popFront / front / back ...... O(1)
//   popBack .............................................. O(n) (must find the node before tail)
//   at / insertAt / removeAt / contains / findIf ......... O(n)

#include <cstddef>
#include <string>

using namespace std;

template <typename T>
class SinglyLinkedList
{
private:
    struct Node
    {
        T data;
        Node* next;

        Node(const T& value, Node* nextNode)
            : data(value), next(nextNode)
        {
        }
    };

public:
    // ----- iterators (forward only) -----

    class Iterator
    {
    public:
        explicit Iterator(Node* node) : node_(node) {}
        T& operator*() const { return node_->data; }
        T* operator->() const { return &node_->data; }
        Iterator& operator++()
        {
            node_ = node_->next;
            return *this;
        }
        bool operator==(const Iterator& other) const { return node_ == other.node_; }
        bool operator!=(const Iterator& other) const { return node_ != other.node_; }

    private:
        Node* node_;
    };

    class ConstIterator
    {
    public:
        explicit ConstIterator(const Node* node) : node_(node) {}
        const T& operator*() const { return node_->data; }
        const T* operator->() const { return &node_->data; }
        ConstIterator& operator++()
        {
            node_ = node_->next;
            return *this;
        }
        bool operator==(const ConstIterator& other) const { return node_ == other.node_; }
        bool operator!=(const ConstIterator& other) const { return node_ != other.node_; }

    private:
        const Node* node_;
    };

    // ----- construction / Rule of Three -----

    SinglyLinkedList()
        : head_(nullptr), tail_(nullptr), size_(0)
    {
    }

    ~SinglyLinkedList()
    {
        clear();
    }

    SinglyLinkedList(const SinglyLinkedList& other)
        : head_(nullptr), tail_(nullptr), size_(0)
    {
        copyFrom(other);
    }

    // GUIDE 1.1: copy first, then clear -> strong exception guarantee
    SinglyLinkedList& operator=(const SinglyLinkedList& other)
    {
        if (this == &other)
            return *this;

        // Copy into a temporary first: if a node allocation throws,
        // *this is left untouched and the temporary cleans itself up.
        SinglyLinkedList temp(other);
        clear();
        head_ = temp.head_;
        tail_ = temp.tail_;
        size_ = temp.size_;
        temp.head_ = nullptr;
        temp.tail_ = nullptr;
        temp.size_ = 0;
        return *this;
    }

    // ----- insertion -----

    void pushFront(const T& value)
    {
        head_ = new Node(value, head_);
        if (tail_ == nullptr)
            tail_ = head_;
        ++size_;
    }

    void pushBack(const T& value)
    {
        Node* node = new Node(value, nullptr);
        if (tail_ == nullptr)
        {
            head_ = node;
            tail_ = node;
        }
        else
        {
            tail_->next = node;
            tail_ = node;
        }
        ++size_;
    }

    // Inserts value so that it ends up at position index (0 = front, size() = back).
    void insertAt(size_t index, const T& value)
    {
        if (index > size_)
            throw "SinglyLinkedList: index out of range";

        if (index == 0)
        {
            pushFront(value);
            return;
        }
        if (index == size_)
        {
            pushBack(value);
            return;
        }

        Node* before = nodeAt(index - 1);
        before->next = new Node(value, before->next);
        ++size_;
    }

    // ----- removal -----

    void popFront()
    {
        checkNotEmpty();
        Node* oldHead = head_;
        head_ = head_->next;
        if (head_ == nullptr)
            tail_ = nullptr;
        delete oldHead;
        --size_;
    }

    void popBack()
    {
        checkNotEmpty();
        if (head_ == tail_)
        {
            delete head_;
            head_ = nullptr;
            tail_ = nullptr;
            size_ = 0;
            return;
        }

        Node* beforeTail = nodeAt(size_ - 2);
        delete tail_;
        tail_ = beforeTail;
        tail_->next = nullptr;
        --size_;
    }

    void removeAt(size_t index)
    {
        if (index >= size_)
            throw "SinglyLinkedList: index out of range";

        if (index == 0)
        {
            popFront();
            return;
        }

        Node* before = nodeAt(index - 1);
        unlinkAfter(before);
    }

    // Removes the first element equal to value. Returns false if none was found.
    bool removeFirst(const T& value)
    {
        return removeIf([&value](const T& element) { return element == value; });
    }

    // Removes the first element for which predicate(element) is true.
    // Returns false if no element matched.
    template <typename Predicate>
    bool removeIf(Predicate predicate)
    {
        if (head_ == nullptr)
            return false;

        if (predicate(head_->data))
        {
            popFront();
            return true;
        }

        for (Node* before = head_; before->next != nullptr; before = before->next)
        {
            if (predicate(before->next->data))
            {
                unlinkAfter(before);
                return true;
            }
        }
        return false;
    }

    // Removes every element for which predicate(element) is true, in one
    // pass over the list (O(n)). Returns how many were removed.
    // GUIDE 1.3 / 2.6: Solver::applyGuess filters the candidate list with this
    template <typename Predicate>
    size_t removeAllIf(Predicate predicate)
    {
        size_t removed = 0;
        while (head_ != nullptr && predicate(head_->data))
        {
            popFront();
            ++removed;
        }
        if (head_ == nullptr)
            return removed;

        Node* before = head_;
        while (before->next != nullptr)
        {
            if (predicate(before->next->data))
            {
                unlinkAfter(before); // keeps tail_ correct when the last node goes
                ++removed;
            }
            else
            {
                before = before->next;
            }
        }
        return removed;
    }

    void clear()
    {
        Node* current = head_;
        while (current != nullptr)
        {
            Node* next = current->next;
            delete current;
            current = next;
        }
        head_ = nullptr;
        tail_ = nullptr;
        size_ = 0;
    }

    // ----- access -----

    T& front()
    {
        checkNotEmpty();
        return head_->data;
    }

    const T& front() const
    {
        checkNotEmpty();
        return head_->data;
    }

    T& back()
    {
        checkNotEmpty();
        return tail_->data;
    }

    const T& back() const
    {
        checkNotEmpty();
        return tail_->data;
    }

    T& at(size_t index)
    {
        if (index >= size_)
            throw "SinglyLinkedList: index out of range";
        return nodeAt(index)->data;
    }

    const T& at(size_t index) const
    {
        if (index >= size_)
            throw "SinglyLinkedList: index out of range";
        return nodeAt(index)->data;
    }

    // ----- searching -----

    bool contains(const T& value) const
    {
        return findIf([&value](const T& element) { return element == value; }) != nullptr;
    }

    // Returns a pointer to the first element for which predicate(element) is true,
    // or nullptr. The hash table uses this to find an entry by its key.
    template <typename Predicate>
    T* findIf(Predicate predicate)
    {
        for (Node* node = head_; node != nullptr; node = node->next)
        {
            if (predicate(node->data))
                return &node->data;
        }
        return nullptr;
    }

    template <typename Predicate>
    const T* findIf(Predicate predicate) const
    {
        for (const Node* node = head_; node != nullptr; node = node->next)
        {
            if (predicate(node->data))
                return &node->data;
        }
        return nullptr;
    }

    // ----- queries -----

    size_t size() const { return size_; }
    bool empty() const { return size_ == 0; }

    // ----- iteration (front to back) -----

    Iterator begin() { return Iterator(head_); }
    Iterator end() { return Iterator(nullptr); }
    ConstIterator begin() const { return ConstIterator(head_); }
    ConstIterator end() const { return ConstIterator(nullptr); }

private:
    Node* head_;
    Node* tail_;
    size_t size_;

    // Caller guarantees index < size_.
    Node* nodeAt(size_t index) const
    {
        Node* node = head_;
        for (size_t i = 0; i < index; ++i)
            node = node->next;
        return node;
    }

    // Deletes the node after `before`. Caller guarantees before->next exists.
    void unlinkAfter(Node* before)
    {
        Node* doomed = before->next;
        before->next = doomed->next;
        if (doomed == tail_)
            tail_ = before;
        delete doomed;
        --size_;
    }

    void copyFrom(const SinglyLinkedList& other)
    {
        for (const Node* node = other.head_; node != nullptr; node = node->next)
            pushBack(node->data);
    }

    void checkNotEmpty() const
    {
        if (size_ == 0)
            throw "SinglyLinkedList: the list is empty";
    }
};

#endif // WORDLE_DSA_SINGLY_LINKED_LIST_H
