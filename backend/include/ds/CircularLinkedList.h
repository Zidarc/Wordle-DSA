#ifndef WORDLE_DSA_CIRCULAR_LINKED_LIST_H
#define WORDLE_DSA_CIRCULAR_LINKED_LIST_H

// CircularLinkedList<T>: a singly linked list whose last node points back to the first.
//
// Used for: secret-word rotation (WordPicker, Day 6). The shuffled words sit in
// a cycle and a cursor marks the current word. Each new game calls advance(),
// so no word repeats until every word has been used once. When the cursor is
// back at the start (cursorAtStart()), the full cycle is complete and the
// WordPicker can reshuffle.
//
//     ┌──► BRAVE ──► CRANE ──► SLATE ──► PLUMB ──┐
//     └──────────────────────────────────────────┘
//                 ↑
//              cursor
//
// Only a tail pointer is stored: the head is always tail_->next, so both
// pushFront and pushBack are O(1).
//
// Rule of Three: the list owns its nodes, so it defines a destructor, a deep-copying
// copy constructor and a deep-copying copy assignment operator. A copy keeps
// the cursor at the same position.
//
// Operations on an empty list throw std::out_of_range.
//
// Complexity:
//   pushFront / pushBack / popFront / front / back ...... O(1)
//   current / advance / resetCursor / cursorAtStart ..... O(1)
//   contains / removeFirst ................................ O(n)

#include <cstddef>
#include <stdexcept>
#include <string>

template <typename T>
class CircularLinkedList
{
private:
    struct Node
    {
        T data;
        Node* next;

        explicit Node(const T& value)
            : data(value), next(nullptr)
        {
        }
    };

public:
    // ----- iteration: exactly one lap, starting at the head -----

    class ConstIterator
    {
    public:
        ConstIterator(const Node* node, std::size_t remaining) : node_(node), remaining_(remaining) {}
        const T& operator*() const { return node_->data; }
        const T* operator->() const { return &node_->data; }
        ConstIterator& operator++()
        {
            node_ = node_->next;
            --remaining_;
            return *this;
        }
        // In a cycle every node has a successor, so "end" is defined by how
        // many steps are left in the lap, not by reaching a null pointer.
        bool operator==(const ConstIterator& other) const { return remaining_ == other.remaining_; }
        bool operator!=(const ConstIterator& other) const { return remaining_ != other.remaining_; }

    private:
        const Node* node_;
        std::size_t remaining_;
    };

    // ----- construction / Rule of Three -----

    CircularLinkedList()
        : tail_(nullptr), cursor_(nullptr), size_(0)
    {
    }

    ~CircularLinkedList()
    {
        clear();
    }

    CircularLinkedList(const CircularLinkedList& other)
        : tail_(nullptr), cursor_(nullptr), size_(0)
    {
        copyFrom(other);
    }

    CircularLinkedList& operator=(const CircularLinkedList& other)
    {
        if (this == &other)
            return *this;

        CircularLinkedList temp(other); // copy first, so a failed copy leaves *this intact
        clear();
        tail_ = temp.tail_;
        cursor_ = temp.cursor_;
        size_ = temp.size_;
        temp.tail_ = nullptr;
        temp.cursor_ = nullptr;
        temp.size_ = 0;
        return *this;
    }

    // ----- insertion -----

    // Adds value at the end of the cycle (just before the head).
    void pushBack(const T& value)
    {
        insertAfterTail(value);
        tail_ = tail_->next; // the new node becomes the tail
    }

    // Adds value at the start of the cycle (it becomes the new head).
    void pushFront(const T& value)
    {
        insertAfterTail(value); // the tail stays, so the new node is the head
    }

    // ----- removal -----

    void popFront()
    {
        checkNotEmpty("popFront");
        Node* head = tail_->next;

        if (head == tail_)
        {
            delete head;
            tail_ = nullptr;
            cursor_ = nullptr;
            size_ = 0;
            return;
        }

        if (cursor_ == head)
            cursor_ = head->next;
        tail_->next = head->next;
        delete head;
        --size_;
    }

    // Removes the first element (starting from the head) equal to value.
    // If the cursor was on that node, it moves on to the next one.
    bool removeFirst(const T& value)
    {
        if (tail_ == nullptr)
            return false;

        Node* before = tail_;
        for (std::size_t i = 0; i < size_; ++i)
        {
            Node* node = before->next;
            if (node->data == value)
            {
                if (size_ == 1)
                {
                    delete node;
                    tail_ = nullptr;
                    cursor_ = nullptr;
                    size_ = 0;
                    return true;
                }
                if (cursor_ == node)
                    cursor_ = node->next;
                if (tail_ == node)
                    tail_ = before;
                before->next = node->next;
                delete node;
                --size_;
                return true;
            }
            before = node;
        }
        return false;
    }

    void clear()
    {
        if (tail_ != nullptr)
        {
            Node* current = tail_->next;
            tail_->next = nullptr; // break the cycle so the loop below stops
            while (current != nullptr)
            {
                Node* next = current->next;
                delete current;
                current = next;
            }
        }
        tail_ = nullptr;
        cursor_ = nullptr;
        size_ = 0;
    }

    // ----- access -----

    const T& front() const
    {
        checkNotEmpty("front");
        return tail_->next->data;
    }

    const T& back() const
    {
        checkNotEmpty("back");
        return tail_->data;
    }

    bool contains(const T& value) const
    {
        for (const T& element : *this)
        {
            if (element == value)
                return true;
        }
        return false;
    }

    // ----- rotation cursor -----

    // The element the cursor is on. A new list's cursor starts on the head.
    const T& current() const
    {
        checkNotEmpty("current");
        return cursor_->data;
    }

    // Moves the cursor one step around the cycle (the last element wraps to the first)
    // and returns the element it lands on.
    const T& advance()
    {
        checkNotEmpty("advance");
        cursor_ = cursor_->next;
        return cursor_->data;
    }

    void resetCursor()
    {
        cursor_ = (tail_ == nullptr) ? nullptr : tail_->next;
    }

    // True when the cursor is on the head, i.e. a full lap has just been completed
    // (or no advance has happened yet).
    bool cursorAtStart() const
    {
        return tail_ != nullptr && cursor_ == tail_->next;
    }

    // ----- queries -----

    std::size_t size() const { return size_; }
    bool empty() const { return size_ == 0; }

    ConstIterator begin() const { return ConstIterator(tail_ == nullptr ? nullptr : tail_->next, size_); }
    ConstIterator end() const { return ConstIterator(nullptr, 0); }

private:
    Node* tail_;   // last node; tail_->next is the head
    Node* cursor_; // current position for rotation
    std::size_t size_;

    // Links a new node between the tail and the head. The caller decides
    // whether that node becomes the new tail (pushBack) or the new head (pushFront).
    void insertAfterTail(const T& value)
    {
        Node* node = new Node(value);
        if (tail_ == nullptr)
        {
            node->next = node; // a single node points to itself
            tail_ = node;
            cursor_ = node;
        }
        else
        {
            node->next = tail_->next;
            tail_->next = node;
        }
        ++size_;
    }

    void copyFrom(const CircularLinkedList& other)
    {
        if (other.tail_ == nullptr)
            return;

        try
        {
            std::size_t cursorIndex = 0;
            const Node* node = other.tail_->next;
            for (std::size_t i = 0; i < other.size_; ++i)
            {
                if (node == other.cursor_)
                    cursorIndex = i;
                pushBack(node->data);
                node = node->next;
            }

            // Put our cursor at the same position as the other list's cursor.
            cursor_ = tail_->next;
            for (std::size_t i = 0; i < cursorIndex; ++i)
                cursor_ = cursor_->next;
        }
        catch (...)
        {
            clear();
            throw;
        }
    }

    void checkNotEmpty(const char* operation) const
    {
        if (size_ == 0)
            throw std::out_of_range(std::string("CircularLinkedList::") + operation + " on an empty list");
    }
};

#endif // WORDLE_DSA_CIRCULAR_LINKED_LIST_H
