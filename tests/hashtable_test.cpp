#include <gtest/gtest.h>
#include "hashtable.hpp"

TEST(HashTableTest, InsertSearchRemove) {
    HashTable table(3);
    EXPECT_TRUE(table.insert("TK-25-111111-2023", 10));
    EXPECT_TRUE(table.insert("TK-25-222222-2024", 20));

    std::size_t listIndex = 0;
    int steps = 0;
    EXPECT_TRUE(table.search("TK-25-111111-2023", listIndex, steps));
    EXPECT_EQ(listIndex, 10u);

    std::size_t removedIndex = 0;
    EXPECT_TRUE(table.remove("TK-25-111111-2023", removedIndex));
    EXPECT_EQ(removedIndex, 10u);
    EXPECT_FALSE(table.search("TK-25-111111-2023", listIndex, steps));

    EXPECT_TRUE(table.search("TK-25-222222-2024", listIndex, steps));
    EXPECT_EQ(listIndex, 20u);
    table.clear();
    EXPECT_FALSE(table.search("TK-25-222222-2024", listIndex, steps));
}

TEST(HashTableTest, InsertDuplicateAndRehash) {
    HashTable table(3);
    EXPECT_TRUE(table.insert("TK-25-111111-2023", 0));
    EXPECT_FALSE(table.insert("TK-25-111111-2023", 1));

    for (int i = 0; i < 10; ++i) {
        std::string key = "TK-25-" + std::to_string(100000 + i);
        table.insert(key, static_cast<std::size_t>(i));
    }

    std::size_t listIndex = 0;
    int steps = 0;
    EXPECT_TRUE(table.search("TK-25-111111-2023", listIndex, steps));
    EXPECT_EQ(listIndex, 0u);
}

TEST(HashTableTest, DumpToStringContainsEntries) {
    HashTable table(5);
    ASSERT_TRUE(table.insert("HX-001", 10));
    ASSERT_TRUE(table.insert("HX-002", 20));

    std::string dump = table.toString();
    EXPECT_NE(dump.find("HashTable dump"), std::string::npos);
    EXPECT_NE(dump.find("HX-001"), std::string::npos);
    EXPECT_NE(dump.find("HX-002"), std::string::npos);
    EXPECT_EQ(dump.find("empty"), std::string::npos);
    EXPECT_NE(dump.find("index=10"), std::string::npos);
}
