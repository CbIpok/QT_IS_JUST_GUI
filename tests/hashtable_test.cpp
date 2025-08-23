#include <gtest/gtest.h>
#include "hashtable.hpp"
#include <vector>

TEST(HashTableTest, InsertSearchRemove) {
    HashTable table(3);
    Record r1{"Alice", 1, "Street", 111, 10};
    Record r2{"Bob", 2, "Ave", 222, 20};
    EXPECT_TRUE(table.insert(r1));
    EXPECT_TRUE(table.insert(r2));
    size_t idx; int steps;
    EXPECT_TRUE(table.search("Alice", 1, idx, steps));
    EXPECT_EQ(table.getOriginalLine(idx), 10);
    EXPECT_TRUE(table.remove(r1));
    EXPECT_FALSE(table.search("Alice", 1, idx, steps));
    EXPECT_TRUE(table.search("Bob", 2, idx, steps));
    table.clear();
    EXPECT_FALSE(table.search("Bob", 2, idx, steps));
}

TEST(HashTableTest, InsertDuplicateAndRehash) {
    HashTable table(3);
    Record r{"Alice", 1, "Street", 111, 0};
    EXPECT_TRUE(table.insert(r));
    EXPECT_FALSE(table.insert(r));

    std::vector<Record> records;
    for (int i = 0; i < 10; ++i) {
        records.push_back({"Name" + std::to_string(i), i, "S", i, i});
    }
    for (const auto& rec : records) {
        table.insert(rec);
    }
    size_t idx; int steps;
    for (const auto& rec : records) {
        EXPECT_TRUE(table.search(rec.fio, rec.applicationNumber, idx, steps));
    }
}
