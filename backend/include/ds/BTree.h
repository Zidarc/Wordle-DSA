#ifndef WORDLE_DSA_BTREE_H
#define WORDLE_DSA_BTREE_H

// BTree<T, Order>: a B-tree where every node has at most Order children and
// at most Order - 1 keys. With the default Order = 3 it is a 2-3 tree: every
// internal node has 2 or 3 children and holds 1 or 2 keys.
//
// Used for: the sixth dictionary lookup method. The AVL tree stores one word
// per node; the 2-3 tree stores up to two per node, so it is shorter and a
// lookup visits fewer nodes (though it compares against more keys per node).
//
//                    [ GHOST | PLUMB ]
//                   /        |        \                    (diagram)
//        [ APPLE ]   [ LEMON ]   [ STONE | ZEBRA ]
//
// Rules kept after every insert:
//   - keys inside a node are sorted
//   - a node other than the root has at least ceil(Order/2) - 1 keys
//   - every leaf is on the same level (the tree only grows at the root)
//
// Insert (bottom-up split):
//   1. Walk down to the leaf where the key belongs and add it there.
//   2. If that node now holds Order keys (one too many), split it: the middle
//      key moves up into the parent, the keys left and right of it become two
//      nodes. The parent may overflow in turn and is split the same way.
//   3. If the root splits, a new root is made above it, and the height grows by 1.
//
// Duplicates are ignored.
//
// Rule of Three: the tree owns its nodes, so it defines a destructor, a
// deep-copying copy constructor and a deep-copying copy assignment operator.
//
// Complexity (n keys, m = Order):
//   insert / contains .... O(m log_m n)  (log_m n levels, up to m keys per node)
//   height ............... O(log_m n)
//   copy / destroy ....... O(n)

#include <cstddef>
#include <string>

template <typename T, int Order = 3>
class BTree
{
    static_assert(Order >= 3, "a B-tree needs an order of at least 3");

private:
    struct Node
    {
        // One extra slot each, so a node can hold Order keys for a moment
        // before it is split.
        T keys[Order];
        Node* children[Order + 1];
        int count;
        bool leaf;

        explicit Node(bool isLeaf)
            : count(0), leaf(isLeaf)
        {
            for (int i = 0; i <= Order; ++i)
                children[i] = nullptr;
        }
    };

public:
    static constexpr int kMaxKeys = Order - 1;
    static constexpr int kMinKeys = (Order + 1) / 2 - 1; // ceil(Order/2) - 1

    BTree()
        : root_(nullptr), size_(0), nodeCount_(0)
    {
    }

    ~BTree()
    {
        destroy(root_);
    }

    BTree(const BTree& other)
        : root_(nullptr), size_(other.size_), nodeCount_(other.nodeCount_)
    {
        root_ = copyTree(other.root_);
    }

    BTree& operator=(const BTree& other)
    {
        if (this == &other)
            return *this;

        Node* copy = copyTree(other.root_); // copy first: *this is unchanged if it throws
        destroy(root_);
        root_ = copy;
        size_ = other.size_;
        nodeCount_ = other.nodeCount_;
        return *this;
    }

    // Returns false if the key was already in the tree.
    bool insert(const T& key)
    {
        if (root_ == nullptr)
        {
            root_ = new Node(true);
            ++nodeCount_;
        }

        bool added = insertInto(root_, key);
        if (!added)
            return false;
        ++size_;

        if (root_->count == Order) // the root overflowed: grow a new root
        {
            Node* newRoot = new Node(false);
            ++nodeCount_;
            newRoot->children[0] = root_;
            splitChild(newRoot, 0);
            root_ = newRoot;
        }
        return true;
    }

    bool contains(const T& key) const
    {
        int comparisons = 0;
        int nodesVisited = 0;
        return search(key, comparisons, nodesVisited);
    }

    bool contains(const T& key, int& comparisons) const
    {
        int nodesVisited = 0;
        return search(key, comparisons, nodesVisited);
    }

    // Counts key comparisons and the nodes visited on the way down.
    bool search(const T& key, int& comparisons, int& nodesVisited) const
    {
        const Node* node = root_;
        while (node != nullptr)
        {
            ++nodesVisited;
            int i = 0;
            while (i < node->count)
            {
                ++comparisons;
                if (key == node->keys[i])
                    return true;
                ++comparisons;
                if (key < node->keys[i])
                    break;
                ++i;
            }
            node = node->leaf ? nullptr : node->children[i];
        }
        return false;
    }

    void clear()
    {
        destroy(root_);
        root_ = nullptr;
        size_ = 0;
        nodeCount_ = 0;
    }

    std::size_t size() const { return size_; }
    bool empty() const { return size_ == 0; }
    std::size_t nodeCount() const { return nodeCount_; }

    // Levels in the tree (empty tree 0, a single leaf 1). Every leaf is at
    // this depth, so following the leftmost child is enough.
    int height() const
    {
        int h = 0;
        for (const Node* node = root_; node != nullptr; node = node->leaf ? nullptr : node->children[0])
            ++h;
        return h;
    }

    // Keys in sorted order (recursive in-order walk). The array is cleared first.
    template <typename Array>
    void toSortedArray(Array& result) const
    {
        while (result.size() > 0)
            result.popBack();
        collect(root_, result);
    }

    // Checks every B-tree rule: sorted keys, key counts within limits, child
    // keys between their separators, and all leaves on the same level.
    bool isValid() const
    {
        if (root_ == nullptr)
            return size_ == 0;
        int leafDepth = -1;
        std::size_t keys = 0;
        std::size_t nodes = 0;
        if (!validate(root_, nullptr, nullptr, 1, leafDepth, keys, nodes, true))
            return false;
        return keys == size_ && nodes == nodeCount_;
    }

private:
    Node* root_;
    std::size_t size_;
    std::size_t nodeCount_;

    // Adds key in the subtree under node. A child that overflows is split
    // here, so on return only node itself may hold Order keys.
    bool insertInto(Node* node, const T& key)
    {
        int i = 0;
        while (i < node->count && node->keys[i] < key)
            ++i;
        if (i < node->count && node->keys[i] == key)
            return false;

        if (node->leaf)
        {
            for (int j = node->count; j > i; --j)
                node->keys[j] = node->keys[j - 1];
            node->keys[i] = key;
            ++node->count;
            return true;
        }

        if (!insertInto(node->children[i], key))
            return false;
        if (node->children[i]->count == Order)
            splitChild(node, i);
        return true;
    }

    // parent->children[index] holds Order keys. Its middle key moves up into
    // parent, and its right half becomes a new node at children[index + 1].
    void splitChild(Node* parent, int index)
    {
        Node* full = parent->children[index];
        const int mid = Order / 2;

        Node* right = new Node(full->leaf);
        ++nodeCount_;
        right->count = full->count - mid - 1;
        for (int j = 0; j < right->count; ++j)
            right->keys[j] = full->keys[mid + 1 + j];
        if (!full->leaf)
        {
            for (int j = 0; j <= right->count; ++j)
            {
                right->children[j] = full->children[mid + 1 + j];
                full->children[mid + 1 + j] = nullptr;
            }
        }
        T middle = full->keys[mid];
        full->count = mid;

        // Open a gap in parent at index for the middle key and the new child.
        for (int j = parent->count; j > index; --j)
        {
            parent->keys[j] = parent->keys[j - 1];
            parent->children[j + 1] = parent->children[j];
        }
        parent->keys[index] = middle;
        parent->children[index + 1] = right;
        ++parent->count;
    }

    template <typename Array>
    void collect(const Node* node, Array& result) const
    {
        if (node == nullptr)
            return;
        for (int i = 0; i < node->count; ++i)
        {
            if (!node->leaf)
                collect(node->children[i], result);
            result.pushBack(node->keys[i]);
        }
        if (!node->leaf)
            collect(node->children[node->count], result);
    }

    bool validate(const Node* node, const T* low, const T* high, int depth, int& leafDepth,
                  std::size_t& keys, std::size_t& nodes, bool isRoot) const
    {
        ++nodes;
        keys += static_cast<std::size_t>(node->count);
        if (node->count > kMaxKeys || node->count < (isRoot ? 1 : kMinKeys))
            return false;
        for (int i = 0; i < node->count; ++i)
        {
            if (i > 0 && !(node->keys[i - 1] < node->keys[i]))
                return false;
            if (low != nullptr && !(*low < node->keys[i]))
                return false;
            if (high != nullptr && !(node->keys[i] < *high))
                return false;
        }

        if (node->leaf)
        {
            if (leafDepth == -1)
                leafDepth = depth;
            return leafDepth == depth;
        }

        for (int i = 0; i <= node->count; ++i)
        {
            const Node* child = node->children[i];
            if (child == nullptr)
                return false;
            const T* childLow = i == 0 ? low : &node->keys[i - 1];
            const T* childHigh = i == node->count ? high : &node->keys[i];
            if (!validate(child, childLow, childHigh, depth + 1, leafDepth, keys, nodes, false))
                return false;
        }
        return true;
    }

    void destroy(Node* node)
    {
        if (node == nullptr)
            return;
        if (!node->leaf)
        {
            for (int i = 0; i <= node->count; ++i)
                destroy(node->children[i]);
        }
        delete node;
    }

    Node* copyTree(const Node* node)
    {
        if (node == nullptr)
            return nullptr;
        Node* copy = new Node(node->leaf);
        copy->count = node->count;
        for (int i = 0; i < node->count; ++i)
            copy->keys[i] = node->keys[i];
        if (!node->leaf)
        {
            for (int i = 0; i <= node->count; ++i)
                copy->children[i] = copyTree(node->children[i]);
        }
        return copy;
    }
};

#endif // WORDLE_DSA_BTREE_H
