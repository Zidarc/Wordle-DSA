#ifndef WORDLE_DSA_DOUBLY_LINKED_LIST_H
#define WORDLE_DSA_DOUBLY_LINKED_LIST_H

// DoublyLinkedList<T>: every node links to both its previous and next node.
//
// Used for: game replay (Day 8). After a game ends, the player steps forward
// and backward through the guesses. The bidirectional iterator below supports
// both ++ and --, and decrementing end() lands on the last element, so a
// replay can start from either end.
//
// Rule of Three: the list owns its nodes, so it defines a destructor, a deep-copying
// copy constructor and a deep-copying copy assignment operator.
//
// Operations on an empty list and invalid indexes throw std::out_of_range.
//
// Complexity:
//   pushFront / pushBack / popFront / popBack / front / back ... O(1)
//   at / insertAt / removeAt ................................... O(n), walking from the nearer end
//   contains / removeFirst ...................................... O(n)

#include <cstddef>
#include <stdexcept>
#include <string>

template <typename T>
class DoublyLinkedList
{
private:
    struct Node
    {
        T data;
        Node* prev;
        Node* next;

        Node(const T& value, Node* prevNode, Node* nextNode)
            : data(value), prev(prevNode), next(nextNode)
        {
        }
    };

public:
    // ----- bidirectional iterators -----

    class Iterator
    {
    public:
        Iterator(Node* node, const DoublyLinkedList* owner) : node_(node), owner_(owner) {}
        T& operator*() const { return node_->data; }
        T* operator->() const { return &node_->data; }
        Iterator& operator++()
        {
            node_ = node_->next;
            return *this;
        }
        // From end(), -- moves to the last element.
        Iterator& operator--()
        {
            node_ = (node_ == nullptr) ? owner_->tail_ : node_->prev;
            return *this;
        }
        bool operator==(const Iterator& other) const { return node_ == other.node_; }
        bool operator!=(const Iterator& other) const { return node_ != other.node_; }

    private:
        Node* node_;
        const DoublyLinkedList* owner_;
    };

    class ConstIterator
    {
    public:
        ConstIterator(const Node* node, const DoublyLinkedList* owner) : node_(node), owner_(owner) {}
        const T& operator*() const { return node_->data; }
        const T* operator->() const { return &node_->data; }
        ConstIterator& operator++()
        {
            node_ = node_->next;
            return *this;
        }
        ConstIterator& operator--()
        {
            node_ = (node_ == nullptr) ? owner_->tail_ : node_->prev;
            return *this;
        }
        bool operator==(const ConstIterator& other) const { return node_ == other.node_; }
        bool operator!=(const ConstIterator& other) const { return node_ != other.node_; }

    private:
        const Node* node_;
        const DoublyLinkedList* owner_;
    };

    // ----- construction / Rule of Three -----

    DoublyLinkedList()
        : head_(nullptr), tail_(nullptr), size_(0)
    {
    }

    ~DoublyLinkedList()
    {
        clear();
    }

    DoublyLinkedList(const DoublyLinkedList& other)
        : head_(nullptr), tail_(nullptr), size_(0)
    {
        try
        {
            for (const Node* node = other.head_; node != nullptr; node = node->next)
                pushBack(node->data);
        }
        catch (...)
        {
            clear();
            throw;
        }
    }

    DoublyLinkedList& operator=(const DoublyLinkedList& other)
    {
        if (this == &other)
            return *this;

        DoublyLinkedList temp(other); // copy first, so a failed copy leaves *this intact
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
        Node* node = new Node(value, nullptr, head_);
        if (head_ == nullptr)
            tail_ = node;
        else
            head_->prev = node;
        head_ = node;
        ++size_;
    }

    void pushBack(const T& value)
    {
        Node* node = new Node(value, tail_, nullptr);
        if (tail_ == nullptr)
            head_ = node;
        else
            tail_->next = node;
        tail_ = node;
        ++size_;
    }

    // Inserts value so that it ends up at position index (0 = front, size() = back).
    void insertAt(std::size_t index, const T& value)
    {
        if (index > size_)
            throw std::out_of_range(indexMessage("insertAt", index, size_ + 1));

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

        Node* after = nodeAt(index);
        Node* before = after->prev;
        Node* node = new Node(value, before, after);
        before->next = node;
        after->prev = node;
        ++size_;
    }

    // ----- removal -----

    void popFront()
    {
        checkNotEmpty("popFront");
        unlink(head_);
    }

    void popBack()
    {
        checkNotEmpty("popBack");
        unlink(tail_);
    }

    void removeAt(std::size_t index)
    {
        if (index >= size_)
            throw std::out_of_range(indexMessage("removeAt", index, size_));
        unlink(nodeAt(index));
    }

    // Removes the first element equal to value. Returns false if none was found.
    bool removeFirst(const T& value)
    {
        for (Node* node = head_; node != nullptr; node = node->next)
        {
            if (node->data == value)
            {
                unlink(node);
                return true;
            }
        }
        return false;
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
        checkNotEmpty("front");
        return head_->data;
    }

    const T& front() const
    {
        checkNotEmpty("front");
        return head_->data;
    }

    T& back()
    {
        checkNotEmpty("back");
        return tail_->data;
    }

    const T& back() const
    {
        checkNotEmpty("back");
        return tail_->data;
    }

    T& at(std::size_t index)
    {
        if (index >= size_)
            throw std::out_of_range(indexMessage("at", index, size_));
        return nodeAt(index)->data;
    }

    const T& at(std::size_t index) const
    {
        if (index >= size_)
            throw std::out_of_range(indexMessage("at", index, size_));
        return nodeAt(index)->data;
    }

    bool contains(const T& value) const
    {
        for (const Node* node = head_; node != nullptr; node = node->next)
        {
            if (node->data == value)
                return true;
        }
        return false;
    }

    // ----- queries -----

    std::size_t size() const { return size_; }
    bool empty() const { return size_ == 0; }

    // ----- iteration -----

    Iterator begin() { return Iterator(head_, this); }
    Iterator end() { return Iterator(nullptr, this); }
    ConstIterator begin() const { return ConstIterator(head_, this); }
    ConstIterator end() const { return ConstIterator(nullptr, this); }

private:
    Node* head_;
    Node* tail_;
    std::size_t size_;

    // Walks from whichever end is closer. Caller guarantees index < size_.
    Node* nodeAt(std::size_t index) const
    {
        if (index < size_ / 2)
        {
            Node* node = head_;
            for (std::size_t i = 0; i < index; ++i)
                node = node->next;
            return node;
        }

        Node* node = tail_;
        for (std::size_t i = size_ - 1; i > index; --i)
            node = node->prev;
        return node;
    }

    // Removes node from the list and deletes it. Caller guarantees node is in this list.
    void unlink(Node* node)
    {
        if (node->prev != nullptr)
            node->prev->next = node->next;
        else
            head_ = node->next;

        if (node->next != nullptr)
            node->next->prev = node->prev;
        else
            tail_ = node->prev;

        delete node;
        --size_;
    }

    void checkNotEmpty(const char* operation) const
    {
        if (size_ == 0)
            throw std::out_of_range(std::string("DoublyLinkedList::") + operation + " on an empty list");
    }

    static std::string indexMessage(const char* operation, std::size_t index, std::size_t limit)
    {
        return std::string("DoublyLinkedList::") + operation + ": index " + std::to_string(index) +
               " is out of range (valid: 0 to " +
               (limit == 0 ? std::string("none") : std::to_string(limit - 1)) + ")";
    }
};

#endif // WORDLE_DSA_DOUBLY_LINKED_LIST_H
