#ifndef WORDLE_DSA_AVL_TREE_H
#define WORDLE_DSA_AVL_TREE_H

// AVLTree<T>: a self-balancing Binary Search Tree with rotations.
//
// Used for: dictionary word validation. The entire word list is inserted into
// an AVLTree<std::string> so that any word can be found in O(log n).
//
// The tree rebalances itself after every insertion using four rotation cases:
//   LL (single right rotation)
//   RR (single left rotation)
//   LR (left-right, double rotation)
//   RL (right-left, double rotation)
//
// Duplicates are ignored (insert does nothing if the value already exists).
//
// Rule of Three: the tree owns its nodes, so it defines a destructor, a
// deep-copying copy constructor and a deep-copying copy assignment operator.
//
// Search functions take an optional int& comparison counter so the DSA
// Insights panel can report how many comparisons each lookup needed.
//
// An iterative in-order traversal using Stack<T> is provided (toSortedArray),
// which is used in tests to verify that the tree stays sorted.
//
// Complexity:
//   insert / contains ........ O(log n), guaranteed by the AVL balance
//   height ................... O(1) (stored in each node)
//   toSortedArray ............ O(n)
//   destroy / copy ........... O(n)
//   size / empty ............. O(1)

#include "ds/Stack.h"

#include <cstddef>
#include <string>

template <typename T>
class AVLTree
{
private:
    struct Node
    {
        T data;
        Node* left;
        Node* right;
        int height;

        Node(const T& value)
            : data(value), left(nullptr), right(nullptr), height(1)
        {
        }
    };

public:
    // ----- construction / Rule of Three -----

    AVLTree()
        : root_(nullptr), size_(0)
    {
    }

    ~AVLTree()
    {
        destroy(root_);
    }

    AVLTree(const AVLTree& other)
        : root_(nullptr), size_(0)
    {
        root_ = copyTree(other.root_);
        size_ = other.size_;
    }

    AVLTree& operator=(const AVLTree& other)
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

    // Inserts value into the tree and rebalances if needed.
    // Duplicates are ignored.
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

    // Returns the height of the tree (an empty tree has height 0).
    int height() const
    {
        return nodeHeight(root_);
    }

    // ----- iterative in-order traversal using Stack -----

    // Returns the elements in sorted order using an iterative in-order
    // traversal with our own Stack (simulating recursion). This is used
    // in tests to confirm the tree stays sorted after rotations.
    //
    // The caller provides a SafeArray to be filled. It is cleared first.
    template <typename Array>
    void toSortedArray(Array& result) const
    {
        // Clear whatever the caller passed in.
        while (result.size() > 0)
            result.popBack();

        Stack<const Node*> stack;
        const Node* current = root_;

        while (current != nullptr || !stack.empty())
        {
            // Go as far left as possible, pushing each node.
            while (current != nullptr)
            {
                stack.push(current);
                current = current->left;
            }

            // Visit the top node.
            current = stack.pop();
            result.pushBack(current->data);

            // Move to the right subtree.
            current = current->right;
        }
    }

private:
    Node* root_;
    std::size_t size_;

    // ----- height helpers -----

    static int nodeHeight(const Node* node)
    {
        return node == nullptr ? 0 : node->height;
    }

    static int balanceFactor(const Node* node)
    {
        return node == nullptr ? 0 : nodeHeight(node->left) - nodeHeight(node->right);
    }

    static void updateHeight(Node* node)
    {
        int leftH = nodeHeight(node->left);
        int rightH = nodeHeight(node->right);
        node->height = 1 + (leftH > rightH ? leftH : rightH);
    }

    // ----- rotations -----

    // Single right rotation (LL case).
    //
    //       node             left
    //       /  \             /   \
    //     left   C   =>    A    node
    //     /  \                  /  \
    //    A    B                B    C
    //
    static Node* rotateRight(Node* node)
    {
        Node* left = node->left;
        node->left = left->right;
        left->right = node;

        updateHeight(node);
        updateHeight(left);
        return left;
    }

    // Single left rotation (RR case).
    //
    //     node              right
    //     /  \              /   \
    //    A   right   =>   node   C
    //        /  \         /  \
    //       B    C       A    B
    //
    static Node* rotateLeft(Node* node)
    {
        Node* right = node->right;
        node->right = right->left;
        right->left = node;

        updateHeight(node);
        updateHeight(right);
        return right;
    }

    // Rebalances node if its balance factor is outside [-1, +1].
    static Node* balance(Node* node)
    {
        updateHeight(node);
        int bf = balanceFactor(node);

        // Left-heavy
        if (bf > 1)
        {
            // LR case: left child is right-heavy, so rotate left first.
            if (balanceFactor(node->left) < 0)
                node->left = rotateLeft(node->left);

            // LL case (or after the LR pre-rotation): single right rotation.
            return rotateRight(node);
        }

        // Right-heavy
        if (bf < -1)
        {
            // RL case: right child is left-heavy, so rotate right first.
            if (balanceFactor(node->right) > 0)
                node->right = rotateRight(node->right);

            // RR case (or after the RL pre-rotation): single left rotation.
            return rotateLeft(node);
        }

        return node;
    }

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
        // else: duplicate, do nothing and skip rebalance

        return balance(node);
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
        newNode->height = node->height;
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

#endif // WORDLE_DSA_AVL_TREE_H
