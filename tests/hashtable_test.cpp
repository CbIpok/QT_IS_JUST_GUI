#include <gtest/gtest.h>
#include "hashtable.hpp"
#include <vector>

TEST(HashTableTest, InsertSearchRemove) {
    HashTable table(3);
    Record r1{"TK-25-111111-2023", "Novikova Daria Sergeevna", "BMW", 10};
    Record r2{"TK-25-222222-2024", "Melnikov Igor Pavlovich", "Mercedes", 20};
    EXPECT_TRUE(table.insert(r1));
    EXPECT_TRUE(table.insert(r2));
    size_t idx; int steps;
    EXPECT_TRUE(table.search("TK-25-111111-2023", idx, steps));
    EXPECT_EQ(table.getOriginalLine(idx), 10);
    EXPECT_TRUE(table.remove(r1));
    EXPECT_FALSE(table.search("TK-25-111111-2023", idx, steps));
    EXPECT_TRUE(table.search("TK-25-222222-2024", idx, steps));
    table.clear();
    EXPECT_FALSE(table.search("TK-25-222222-2024", idx, steps));
}

TEST(HashTableTest, InsertDuplicateAndRehash) {
    HashTable table(3);
    Record r{"TK-25-111111-2023", "Novikova Daria Sergeevna", "BMW", 0};
    EXPECT_TRUE(table.insert(r));
    EXPECT_FALSE(table.insert(r));

    std::vector<Record> records;
    for (int i = 0; i < 10; ++i) {
        records.push_back({"TK-25-" + std::to_string(100000 + i),
                          "Name" + std::to_string(i),
                          "Brand", i});
    }
    for (const auto& rec : records) {
        table.insert(rec);
    }
    size_t idx; int steps;
    for (const auto& rec : records) {
        EXPECT_TRUE(table.search(rec.licenseNumber, idx, steps));
    }
}
