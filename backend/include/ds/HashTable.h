#ifndef WORDLE_DSA_HASH_TABLE_H
#define WORDLE_DSA_HASH_TABLE_H

// HashTable<K, V>: a hash table with separate chaining and automatic rehashing.
//
// Used for:
//   - the dictionary: O(1) average "is this word valid?" (Day 6)
//   - the server's active games, keyed by gameId (Day 9)
//
// Layout: a dynamic array of buckets. Each bucket is a SinglyLinkedList of
// (key, value) entries, so keys that hash to the same bucket share a chain
// (separate chaining). New entries go to the front of their chain.
//
// Hash function: KeyHash<K>. For string it is a polynomial rolling hash,
//   h = s[0]*31^(n-1) + s[1]*31^(n-2) + ... + s[n-1]   (mod 2^64)
// computed with Horner's rule. Integer keys hash to their own value.
//
// Rehashing: when an insert pushes the load factor (size / bucketCount) above
// 0.75, the table grows to the next prime at least twice the old bucket count,
// and every entry is re-inserted into its new bucket. Prime bucket counts
// spread the keys more evenly across the buckets.
//
// Search functions take an optional int& comparison counter so the DSA
// Insights panel can show how many key comparisons a lookup needed.
//
// Rule of Three: the table owns its bucket array, so it defines a destructor,
// a deep-copying copy constructor and a deep-copying copy assignment operator.
//
// Complexity (n = entries, b = buckets):
//   insert / contains / find / remove ..... O(1) average, O(n) worst (all in one chain)
//   rehash ................................. O(n + b)
//   longestChain / usedBuckets ............. O(b)
//   copy / clear ........................... O(n + b)

#include "ds/SinglyLinkedList.h"

#include <cstddef>
#include <cstdint>
#include <string>

using namespace std;

// ---------------------------------------------------------------------------
// Hash functions (the table picks the right one by the key's type)
// ---------------------------------------------------------------------------

// Polynomial rolling hash for strings.
// GUIDE 1.9: polynomial rolling hash, h = h*31 + c (Horner)
inline uint64_t hashKey(const string& key)
{
    uint64_t hash = 0;
    for (char c : key)
        hash = hash * 31 + (unsigned char)c; // wraps mod 2^64
    return hash;
}

// Integer keys hash to their own value.
inline uint64_t hashKey(long long key)
{
    return (uint64_t)key;
}

// ---------------------------------------------------------------------------
// HashTable
// ---------------------------------------------------------------------------

template <typename K, typename V>
class HashTable
{
public:
    struct Entry
    {
        K key;
        V value;
    };

    static constexpr size_t kDefaultBucketCount = 11;
    static constexpr double kMaxLoadFactor = 0.75;

    // ----- construction / Rule of Three -----

    explicit HashTable(size_t initialBuckets = kDefaultBucketCount)
        : buckets_(nullptr), bucketCount_(0), size_(0), rehashCount_(0)
    {
        bucketCount_ = nextPrime(initialBuckets < 2 ? 2 : initialBuckets);
        buckets_ = new SinglyLinkedList<Entry>[bucketCount_];
    }

    ~HashTable()
    {
        delete[] buckets_;
    }

    HashTable(const HashTable& other)
        : buckets_(nullptr), bucketCount_(other.bucketCount_), size_(other.size_),
          rehashCount_(other.rehashCount_)
    {
        buckets_ = new SinglyLinkedList<Entry>[bucketCount_];
        for (size_t i = 0; i < bucketCount_; ++i)
            buckets_[i] = other.buckets_[i]; // SinglyLinkedList deep-copies its nodes
    }

    HashTable& operator=(const HashTable& other)
    {
        if (this == &other)
            return *this;

        // Copy into a temporary first: if an allocation throws, *this is untouched.
        HashTable temp(other);
        swapWith(temp);
        return *this; // temp now holds the old buckets and frees them
    }

    // ----- insertion -----

    // Inserts key with value. If the key is already present, its value is
    // replaced and false is returned. Returns true if a new entry was added.
    bool insert(const K& key, const V& value)
    {
        SinglyLinkedList<Entry>& chain = bucketFor(key);
        Entry* existing = chain.findIf([&key](const Entry& e) { return e.key == key; });
        if (existing != nullptr)
        {
            existing->value = value;
            return false;
        }

        chain.pushFront(Entry{key, value});
        ++size_;

        if (loadFactor() > kMaxLoadFactor)
            rehash(nextPrime(bucketCount_ * 2));
        return true;
    }

    // ----- lookup -----

    bool contains(const K& key) const
    {
        return find(key) != nullptr;
    }

    // Same as contains(), but adds the number of key comparisons made to `comparisons`.
    bool contains(const K& key, int& comparisons) const
    {
        const SinglyLinkedList<Entry>& chain = bucketFor(key);
        for (const Entry& entry : chain)
        {
            ++comparisons;
            if (entry.key == key)
                return true;
        }
        return false;
    }

    // Returns a pointer to the value for key, or nullptr if the key is missing.
    V* find(const K& key)
    {
        Entry* entry = bucketFor(key).findIf([&key](const Entry& e) { return e.key == key; });
        return entry == nullptr ? nullptr : &entry->value;
    }

    const V* find(const K& key) const
    {
        const Entry* entry = bucketFor(key).findIf([&key](const Entry& e) { return e.key == key; });
        return entry == nullptr ? nullptr : &entry->value;
    }

    // Returns the value for key. Throws an error if the key is missing.
    V& at(const K& key)
    {
        V* value = find(key);
        if (value == nullptr)
            throw "HashTable::at: key not found";
        return *value;
    }

    const V& at(const K& key) const
    {
        const V* value = find(key);
        if (value == nullptr)
            throw "HashTable::at: key not found";
        return *value;
    }

    // ----- removal -----

    // Removes key. Returns false if it was not in the table.
    bool remove(const K& key)
    {
        bool removed = bucketFor(key).removeIf([&key](const Entry& e) { return e.key == key; });
        if (removed)
            --size_;
        return removed;
    }

    // Removes every entry but keeps the current bucket count.
    void clear()
    {
        for (size_t i = 0; i < bucketCount_; ++i)
            buckets_[i].clear();
        size_ = 0;
    }

    // ----- traversal -----

    // Calls visit(key, value) for every entry, bucket by bucket (no particular order).
    template <typename Visitor>
    void forEach(Visitor visit) const
    {
        for (size_t i = 0; i < bucketCount_; ++i)
        {
            for (const Entry& entry : buckets_[i])
                visit(entry.key, entry.value);
        }
    }

    // ----- statistics (for the DSA Insights panel) -----

    size_t size() const { return size_; }
    bool empty() const { return size_ == 0; }
    size_t bucketCount() const { return bucketCount_; }
    size_t rehashCount() const { return rehashCount_; }

    double loadFactor() const
    {
        return (double)(size_) / (double)(bucketCount_);
    }

    // Length of the longest collision chain.
    size_t longestChain() const
    {
        size_t longest = 0;
        for (size_t i = 0; i < bucketCount_; ++i)
        {
            if (buckets_[i].size() > longest)
                longest = buckets_[i].size();
        }
        return longest;
    }

    // Number of buckets that hold at least one entry.
    size_t usedBuckets() const
    {
        size_t used = 0;
        for (size_t i = 0; i < bucketCount_; ++i)
        {
            if (!buckets_[i].empty())
                ++used;
        }
        return used;
    }

    // Which bucket a key belongs to (exposed so tests can build collisions).
    size_t bucketIndex(const K& key) const
    {
        return (size_t)(hashKey(key) % bucketCount_);
    }

    // ----- rehashing -----

    // Moves every entry into a new array of newBucketCount buckets (rounded up
    // to a prime). Called automatically by insert(); public so it can be shown
    // and tested on its own.
    // GUIDE 1.9: called by insert() when load factor > 0.75
    void rehash(size_t newBucketCount)
    {
        newBucketCount = nextPrime(newBucketCount < 2 ? 2 : newBucketCount);
        SinglyLinkedList<Entry>* newBuckets = new SinglyLinkedList<Entry>[newBucketCount];

        for (size_t i = 0; i < bucketCount_; ++i)
        {
            for (const Entry& entry : buckets_[i])
            {
                size_t index = (size_t)(hashKey(entry.key) % newBucketCount);
                newBuckets[index].pushFront(entry);
            }
        }

        delete[] buckets_;
        buckets_ = newBuckets;
        bucketCount_ = newBucketCount;
        ++rehashCount_;
    }

private:
    SinglyLinkedList<Entry>* buckets_;
    size_t bucketCount_;
    size_t size_;
    size_t rehashCount_;

    SinglyLinkedList<Entry>& bucketFor(const K& key)
    {
        return buckets_[bucketIndex(key)];
    }

    const SinglyLinkedList<Entry>& bucketFor(const K& key) const
    {
        return buckets_[bucketIndex(key)];
    }

    void swapWith(HashTable& other)
    {
        SinglyLinkedList<Entry>* buckets = buckets_;
        buckets_ = other.buckets_;
        other.buckets_ = buckets;

        size_t value = bucketCount_;
        bucketCount_ = other.bucketCount_;
        other.bucketCount_ = value;

        value = size_;
        size_ = other.size_;
        other.size_ = value;

        value = rehashCount_;
        rehashCount_ = other.rehashCount_;
        other.rehashCount_ = value;
    }

    static bool isPrime(size_t n)
    {
        if (n < 2)
            return false;
        if (n % 2 == 0)
            return n == 2;
        for (size_t d = 3; d * d <= n; d += 2)
        {
            if (n % d == 0)
                return false;
        }
        return true;
    }

    // Smallest prime >= n.
    static size_t nextPrime(size_t n)
    {
        while (!isPrime(n))
            ++n;
        return n;
    }
};

#endif // WORDLE_DSA_HASH_TABLE_H
