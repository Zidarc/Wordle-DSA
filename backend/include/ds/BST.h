#ifndef WORDLE_DSA_BST_H
#define WORDLE_DSA_BST_H

// BST<T>: an unbalanced Binary Search Tree.
//
// Used for: the secret word's letters. The game inserts each letter of the
// secret word into a BST<char>, so that "is this letter in the word?" is
// answered in O(log L) where L is the word length (5).
//
// Duplicates are ignored (insert does nothing if the value already exists).
//
// Rule of Three: the tree owns its nodes, so it defines a destructor, a
// deep-copying copy constructor and a deep-copying copy assignment operator.
//
// Search functions take an optional int& comparison counter so the DSA
// Insights panel can report how many comparisons each lookup needed.
//
// Complexity:
//   insert / contains ........ O(h), where h is the height
//   destroy / copy ........... O(n)
//   size / empty ............. O(1)

#include <cstddef>
#include <string>

template <typename T>
class BST
{
private:
    struct Node
    {
        T data;
        Node* left;
        Node* right;

        Node(const T& value)
            : data(value), left(nullptr), right(nullptr)
        {
        }
    };

public:
    // ----- construction / Rule of Three -----

    BST()
        : root_(nullptr), size_(0)
    {
    }

    ~BST()
    {
        destroy(root_);
    }

    BST(const BST& other)
        : root_(nullptr), size_(0)
    {
        root_ = copyTree(other.root_);
        size_ = other.size_;
    }

    BST& operator=(const BST& other)
    {
        if (this == &other)
            return *this;

        // Build the copy first so *this is unchanged if allocation throws.
        Node* newRoot = copyTree(other.root_);
        destroy(root_);
        root_ = newRoot;
        size_ = other.size_;
        return *this;
    }

    // ----- insertion -----

    // Inserts value into the tree. Duplicates are ignored.
    void insert(const T& value)
    {
        root_ = insert(root_, value);
    }

    // ----- searching -----

    // Returns true if the tree contains value.
    bool contains(const T& value) const
    {
        return contains(root_, value);
    }

    // Returns true if the tree contains value, and increments comparisons
    // for every comparison made (for the DSA Insights panel).
    bool contains(const T& value, int& comparisons) const
    {
        return containsCounting(root_, value, comparisons);
    }

    // ----- removal -----

    void clear()
    {
        destroy(root_);
        root_ = nullptr;
        size_ = 0;
    }

    // ----- queries -----

    std::size_t size() const { return size_; }
    bool empty() const { return size_ == 0; }

    // Returns the height of the tree (an empty tree has height 0,
    // a single node has height 1).
    int height() const
    {
        return height(root_);
    }

private:
    Node* root_;
    std::size_t size_;

    // ----- recursive helpers -----

    Node* insert(Node* node, const T& value)
    {
        if (node == nullptr)
        {
            ++size_;
            return new Node(value);
        }

        if (value < node->data)
            node->left = insert(node->left, value);
        else if (node->data < value)
            node->right = insert(node->right, value);
        // else: duplicate, do nothing

        return node;
    }

    bool contains(const Node* node, const T& value) const
    {
        if (node == nullptr)
            return false;
        if (value < node->data)
            return contains(node->left, value);
        if (node->data < value)
            return contains(node->right, value);
        return true; // found
    }

    bool containsCounting(const Node* node, const T& value, int& comparisons) const
    {
        if (node == nullptr)
            return false;

        ++comparisons;
        if (value < node->data)
            return containsCounting(node->left, value, comparisons);

        ++comparisons;
        if (node->data < value)
            return containsCounting(node->right, value, comparisons);

        return true; // found
    }

    int height(const Node* node) const
    {
        if (node == nullptr)
            return 0;
        int leftHeight = height(node->left);
        int rightHeight = height(node->right);
        return 1 + (leftHeight > rightHeight ? leftHeight : rightHeight);
    }

    void destroy(Node* node)
    {
        if (node == nullptr)
            return;
        destroy(node->left);
        destroy(node->right);
        delete node;
    }

    Node* copyTree(const Node* node)
    {
        if (node == nullptr)
            return nullptr;

        Node* newNode = new Node(node->data);
        try
        {
            newNode->left = copyTree(node->left);
            newNode->right = copyTree(node->right);
        }
        catch (...)
        {
            destroy(newNode);
            throw;
        }
        return newNode;
    }
};

#endif // WORDLE_DSA_BST_H
