#include "TestFramework.h"
#include "ds/HashTable.h"

#include <cstddef>
#include <string>

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

// Five-letter test words "AAAAA", "AAAAB", ... built from an index in base 26.
static std::string wordFromIndex(int index)
{
    std::string word(5, 'A');
    for (int pos = 4; pos >= 0; --pos)
    {
        word[pos] = static_cast<char>('A' + index % 26);
        index /= 26;
    }
    return word;
}

// ===========================================================================
// Construction
// ===========================================================================

TEST_CASE(hashTableDefaultConstructedIsEmpty)
{
    HashTable<std::string, int> table;
    CHECK(table.empty());
    CHECK_EQ(table.size(), 0u);
    CHECK_EQ(table.bucketCount(), 11u);
    CHECK_EQ(table.loadFactor(), 0.0);
    CHECK_EQ(table.longestChain(), 0u);
    CHECK_EQ(table.usedBuckets(), 0u);
}

TEST_CASE(hashTableBucketCountIsRoundedUpToPrime)
{
    HashTable<int, int> table(20);
    CHECK_EQ(table.bucketCount(), 23u);

    HashTable<int, int> tiny(0);
    CHECK_EQ(tiny.bucketCount(), 2u);
}

// ===========================================================================
// Insert / search / missing key
// ===========================================================================

TEST_CASE(hashTableInsertAndFind)
{
    HashTable<std::string, int> table;
    CHECK(table.insert("CRANE", 1));
    CHECK(table.insert("SLATE", 2));
    CHECK_EQ(table.size(), 2u);

    CHECK(table.contains("CRANE"));
    CHECK(table.contains("SLATE"));
    CHECK_EQ(table.at("CRANE"), 1);
    CHECK_EQ(*table.find("SLATE"), 2);
}

TEST_CASE(hashTableMissingKey)
{
    HashTable<std::string, int> table;
    table.insert("CRANE", 1);

    CHECK(!table.contains("XQZTV"));
    CHECK(table.find("XQZTV") == nullptr);
    CHECK_THROWS(table.at("XQZTV"));

    HashTable<std::string, int> empty;
    CHECK(!empty.contains("CRANE"));
}

TEST_CASE(hashTableDuplicateKeyReplacesValue)
{
    HashTable<std::string, int> table;
    CHECK(table.insert("CRANE", 1));
    CHECK(!table.insert("CRANE", 99));
    CHECK_EQ(table.size(), 1u);
    CHECK_EQ(table.at("CRANE"), 99);
}

TEST_CASE(hashTableValueCanBeModifiedThroughFind)
{
    HashTable<std::string, int> table;
    table.insert("CRANE", 1);
    *table.find("CRANE") += 10;
    table.at("CRANE") += 5;
    CHECK_EQ(table.at("CRANE"), 16);
}

TEST_CASE(hashTableIsCaseSensitive)
{
    HashTable<std::string, int> table;
    table.insert("CRANE", 1);
    CHECK(!table.contains("crane")); // callers normalise case (Dictionary does this)
}

// ===========================================================================
// Hash function
// ===========================================================================

TEST_CASE(stringHashIsPolynomialRollingHash)
{
    // "AB" = 'A' * 31 + 'B' = 65 * 31 + 66
    CHECK_EQ(hashKey("AB"), static_cast<std::uint64_t>(65 * 31 + 66));
    CHECK_EQ(hashKey(""), static_cast<std::uint64_t>(0));
    // Order matters, unlike a plain sum of characters.
    CHECK(hashKey("ABC") != hashKey("CBA"));
}

TEST_CASE(integerHashIsIdentity)
{
    CHECK_EQ(hashKey(42), static_cast<std::uint64_t>(42));
}

// ===========================================================================
// Collisions
// ===========================================================================

TEST_CASE(hashTableCollisionsShareAChain)
{
    // With 11 buckets and identity hashing, 3, 14 and 25 all land in bucket 3.
    HashTable<int, std::string> table(11);
    table.insert(3, "three");
    table.insert(14, "fourteen");
    table.insert(25, "twenty-five");

    CHECK_EQ(table.bucketIndex(3), table.bucketIndex(14));
    CHECK_EQ(table.bucketIndex(14), table.bucketIndex(25));
    CHECK_EQ(table.usedBuckets(), 1u);
    CHECK_EQ(table.longestChain(), 3u);

    CHECK_EQ(table.at(3), std::string("three"));
    CHECK_EQ(table.at(14), std::string("fourteen"));
    CHECK_EQ(table.at(25), std::string("twenty-five"));
    CHECK(!table.contains(36)); // same bucket, not present
}

TEST_CASE(hashTableComparisonCounterWalksTheChain)
{
    HashTable<int, int> table(11);
    table.insert(3, 0);
    table.insert(14, 0);
    table.insert(25, 0); // chain is now 25 -> 14 -> 3 (new entries go to the front)

    int comparisons = 0;
    CHECK(table.contains(25, comparisons));
    CHECK_EQ(comparisons, 1);

    comparisons = 0;
    CHECK(table.contains(3, comparisons));
    CHECK_EQ(comparisons, 3);

    comparisons = 0;
    CHECK(!table.contains(36, comparisons)); // walks the whole chain
    CHECK_EQ(comparisons, 3);

    comparisons = 0;
    CHECK(!table.contains(5, comparisons)); // empty bucket
    CHECK_EQ(comparisons, 0);
}

// ===========================================================================
// Removal
// ===========================================================================

TEST_CASE(hashTableRemove)
{
    HashTable<int, int> table(11);
    table.insert(3, 30);
    table.insert(14, 140);
    table.insert(25, 250);

    CHECK(table.remove(14)); // middle of the chain
    CHECK_EQ(table.size(), 2u);
    CHECK(!table.contains(14));
    CHECK(table.contains(3));
    CHECK(table.contains(25));

    CHECK(!table.remove(14)); // already gone
    CHECK(!table.remove(99)); // never there
    CHECK_EQ(table.size(), 2u);
}

TEST_CASE(hashTableClearKeepsBuckets)
{
    HashTable<std::string, int> table;
    table.insert("CRANE", 1);
    table.insert("SLATE", 2);
    std::size_t buckets = table.bucketCount();

    table.clear();
    CHECK(table.empty());
    CHECK(!table.contains("CRANE"));
    CHECK_EQ(table.bucketCount(), buckets);

    table.insert("BRAVE", 3); // still usable
    CHECK(table.contains("BRAVE"));
}

// ===========================================================================
// Rehashing
// ===========================================================================

TEST_CASE(hashTableRehashesAboveLoadFactor)
{
    HashTable<int, int> table(11);
    for (int i = 0; i < 8; ++i) // 8 / 11 = 0.727, still under 0.75
        table.insert(i, i);
    CHECK_EQ(table.bucketCount(), 11u);
    CHECK_EQ(table.rehashCount(), 0u);

    table.insert(8, 8); // 9 / 11 = 0.818 -> rehash to nextPrime(22) = 23
    CHECK_EQ(table.bucketCount(), 23u);
    CHECK_EQ(table.rehashCount(), 1u);
    CHECK((table.loadFactor() <= HashTable<int, int>::kMaxLoadFactor));
}

TEST_CASE(hashTableEveryEntryFindableAfterRehashing)
{
    HashTable<std::string, int> table;
    const int count = 2000;
    for (int i = 0; i < count; ++i)
        table.insert(wordFromIndex(i), i);

    CHECK_EQ(table.size(), static_cast<std::size_t>(count));
    CHECK(table.rehashCount() > 0u);
    CHECK((table.loadFactor() <= HashTable<std::string, int>::kMaxLoadFactor));

    bool allFound = true;
    for (int i = 0; i < count; ++i)
    {
        const int* value = table.find(wordFromIndex(i));
        if (value == nullptr || *value != i)
            allFound = false;
    }
    CHECK(allFound);
    CHECK(!table.contains("ZZZZZ"));
}

TEST_CASE(hashTableManualRehashKeepsEntries)
{
    HashTable<int, int> table(11);
    for (int i = 0; i < 5; ++i)
        table.insert(i * 11, i); // all in bucket 0
    CHECK_EQ(table.longestChain(), 5u);

    table.rehash(50); // -> 53 buckets
    CHECK_EQ(table.bucketCount(), 53u);
    CHECK_EQ(table.size(), 5u);
    CHECK_EQ(table.longestChain(), 1u); // the chain is spread out again
    for (int i = 0; i < 5; ++i)
        CHECK_EQ(table.at(i * 11), i);
}

TEST_CASE(hashTableChainsStayShortForWordKeys)
{
    // A good hash keeps chains short on realistic keys.
    HashTable<std::string, int> table;
    for (int i = 0; i < 5000; ++i)
        table.insert(wordFromIndex(i * 7), i);
    CHECK(table.longestChain() <= 8u);
}

// ===========================================================================
// Traversal
// ===========================================================================

TEST_CASE(hashTableForEachVisitsEveryEntryOnce)
{
    HashTable<int, int> table;
    for (int i = 1; i <= 100; ++i)
        table.insert(i, i * 2);

    int visited = 0;
    long keySum = 0;
    long valueSum = 0;
    table.forEach([&](const int& key, const int& value) {
        ++visited;
        keySum += key;
        valueSum += value;
    });
    CHECK_EQ(visited, 100);
    CHECK_EQ(keySum, 5050L);
    CHECK_EQ(valueSum, 10100L);
}

// ===========================================================================
// Rule of Three
// ===========================================================================

TEST_CASE(hashTableCopyConstructorIsDeep)
{
    HashTable<std::string, int> original;
    original.insert("CRANE", 1);
    original.insert("SLATE", 2);

    HashTable<std::string, int> copy(original);
    CHECK_EQ(copy.size(), 2u);
    CHECK_EQ(copy.at("CRANE"), 1);

    copy.at("CRANE") = 100;
    copy.insert("BRAVE", 3);
    copy.remove("SLATE");

    CHECK_EQ(original.at("CRANE"), 1);
    CHECK(!original.contains("BRAVE"));
    CHECK(original.contains("SLATE"));
    CHECK_EQ(original.size(), 2u);
}

TEST_CASE(hashTableCopyAssignmentIsDeep)
{
    HashTable<std::string, int> a;
    for (int i = 0; i < 50; ++i)
        a.insert(wordFromIndex(i), i);

    HashTable<std::string, int> b;
    b.insert("OTHER", 0);
    b = a;

    CHECK_EQ(b.size(), 50u);
    CHECK_EQ(b.bucketCount(), a.bucketCount());
    CHECK(!b.contains("OTHER"));

    b.insert("NEWWD", 1);
    CHECK(!a.contains("NEWWD"));
}

TEST_CASE(hashTableSelfAssignment)
{
    HashTable<std::string, int> table;
    table.insert("CRANE", 1);
    HashTable<std::string, int>& alias = table;
    table = alias;
    CHECK_EQ(table.size(), 1u);
    CHECK_EQ(table.at("CRANE"), 1);
}

int main()
{
    return tf::runAll("HashTable");
}
