#include <gtest/gtest.h>
#include "hashtable.hpp"
#include "DoublyLinkedList.hpp"
#include "driver_record.hpp"
#include <vector>

TEST(HashTableTest, InsertSearchRemove) {
    HashTable table(3);
    DoublyLinkedList<DriverRecord> list;
    DriverRecord r1{"TK-25-111111-2023", "Novikova Daria Sergeevna", "BMW", 10};
    DriverRecord r2{"TK-25-222222-2024", "Melnikov Igor Pavlovich", "Mercedes", 20};
    size_t i1 = list.push_back_index(r1);
    size_t i2 = list.push_back_index(r2);
    EXPECT_TRUE(table.insert(r1.licenseNumber, i1));
    EXPECT_TRUE(table.insert(r2.licenseNumber, i2));
    size_t idx; int steps;
    EXPECT_TRUE(table.get_index("TK-25-111111-2023", idx, steps));
    EXPECT_EQ(idx, i1);
    EXPECT_TRUE(table.remove("TK-25-111111-2023", idx));
    EXPECT_FALSE(table.get_index("TK-25-111111-2023", idx, steps));
    EXPECT_TRUE(table.get_index("TK-25-222222-2024", idx, steps));
    table.clear();
    EXPECT_FALSE(table.get_index("TK-25-222222-2024", idx, steps));
}

TEST(HashTableTest, InsertDuplicateAndRehash) {
    HashTable table(3);
    DoublyLinkedList<DriverRecord> list;
    DriverRecord r{"TK-25-111111-2023", "Novikova Daria Sergeevna", "BMW", 0};
    size_t i = list.push_back_index(r);
    EXPECT_TRUE(table.insert(r.licenseNumber, i));
    EXPECT_FALSE(table.insert(r.licenseNumber, i));

    std::vector<DriverRecord> records;
    for (int j = 0; j < 10; ++j) {
        records.push_back({"TK-25-" + std::to_string(100000 + j),
                          "Name" + std::to_string(j),
                          "Brand", j});
    }
    for (const auto& rec : records) {
        size_t idx = list.push_back_index(rec);
        table.insert(rec.licenseNumber, idx);
    }
    size_t idx; int steps;
    for (const auto& rec : records) {
        EXPECT_TRUE(table.get_index(rec.licenseNumber, idx, steps));
    }
}
