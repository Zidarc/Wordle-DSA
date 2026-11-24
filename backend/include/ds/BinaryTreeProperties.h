#ifndef WORDLE_DSA_BINARY_TREE_PROPERTIES_H
#define WORDLE_DSA_BINARY_TREE_PROPERTIES_H

// Shape properties of a binary tree (lab week 10), shared by BST<T> and
// AVLTree<T>. Each function takes the root node; any node type with `left`
// and `right` pointers works.
//
//   full       every node has 0 or 2 children
//   complete   every level is filled except possibly the last, and the last
//              level is filled from the left (the shape a heap array has)
//   perfect    full, and every leaf is on the same level; then n = 2^h - 1
//
//              full, not complete       complete, not full      perfect
//                    A                         A                   A
//                   / \                       / \                 / \    (diagram)
//                  B   C                     B   C               B   C
//                     / \                   /                   / \ / \  (diagram)
//                    D   E                 D                   D  E F  G
//
// Complexity: every function visits each node once, O(n).
//   countNodes / countLeaves / isFull ..... recursive
//   isComplete ............................ level-order (breadth-first) walk
//                                           with our own Queue

#include "ds/Queue.h"

#include <cstddef>

template <typename Node>
std::size_t btCountNodes(const Node* node)
{
    if (node == nullptr)
        return 0;
    return 1 + btCountNodes(node->left) + btCountNodes(node->right);
}

template <typename Node>
std::size_t btCountLeaves(const Node* node)
{
    if (node == nullptr)
        return 0;
    if (node->left == nullptr && node->right == nullptr)
        return 1;
    return btCountLeaves(node->left) + btCountLeaves(node->right);
}

// Nodes with exactly one child; a full tree has none.
template <typename Node>
std::size_t btCountSingleChildNodes(const Node* node)
{
    if (node == nullptr)
        return 0;
    std::size_t here = ((node->left == nullptr) != (node->right == nullptr)) ? 1 : 0;
    return here + btCountSingleChildNodes(node->left) + btCountSingleChildNodes(node->right);
}

template <typename Node>
int btHeight(const Node* node)
{
    if (node == nullptr)
        return 0;
    int l = btHeight(node->left);
    int r = btHeight(node->right);
    return 1 + (l > r ? l : r);
}

// An empty tree counts as full.
template <typename Node>
bool btIsFull(const Node* node)
{
    if (node == nullptr)
        return true;
    if ((node->left == nullptr) != (node->right == nullptr))
        return false;
    return btIsFull(node->left) && btIsFull(node->right);
}

// Level-order walk: once a missing child has been seen, no later node in the
// walk may have a child.
// GUIDE 1.7: level-order walk with Queue; no child allowed after a gap
template <typename Node>
bool btIsComplete(const Node* root)
{
    if (root == nullptr)
        return true;

    Queue<const Node*> queue;
    queue.enqueue(root);
    bool seenGap = false;

    while (!queue.empty())
    {
        const Node* node = queue.dequeue();
        const Node* children[2] = {node->left, node->right};
        for (const Node* child : children)
        {
            if (child == nullptr)
            {
                seenGap = true;
            }
            else
            {
                if (seenGap)
                    return false;
                queue.enqueue(child);
            }
        }
    }
    return true;
}

// Perfect: n == 2^h - 1.
template <typename Node>
bool btIsPerfect(const Node* root)
{
    std::size_t n = btCountNodes(root);
    int h = btHeight(root);
    return n == (static_cast<std::size_t>(1) << h) - 1;
}

// Summary of all properties, for /api/dsa/trees.
struct TreeShape
{
    std::size_t nodes = 0;
    std::size_t leaves = 0;
    std::size_t internalNodes = 0;
    std::size_t singleChildNodes = 0;
    int height = 0;
    int minPossibleHeight = 0; // ceil(log2(n + 1))
    bool full = true;
    bool complete = true;
    bool perfect = true;
};

template <typename Node>
TreeShape btShape(const Node* root)
{
    TreeShape s;
    s.nodes = btCountNodes(root);
    s.leaves = btCountLeaves(root);
    s.internalNodes = s.nodes - s.leaves;
    s.singleChildNodes = btCountSingleChildNodes(root);
    s.height = btHeight(root);
    while ((static_cast<std::size_t>(1) << s.minPossibleHeight) - 1 < s.nodes)
        ++s.minPossibleHeight;
    s.full = btIsFull(root);
    s.complete = btIsComplete(root);
    s.perfect = btIsPerfect(root);
    return s;
}

#endif // WORDLE_DSA_BINARY_TREE_PROPERTIES_H
