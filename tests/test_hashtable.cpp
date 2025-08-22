#include <gtest/gtest.h>
#include <fstream>
#include <cassert>
#include <fstream>
#include <iostream>
#include "../hashtable.hpp"

TEST(HashTable, BasicOperations) {
    HashTable ht(4);
    Record a{"Ivanov I I", 1, "Street", 12345, 10};
    Record b{"Petrov P P", 2, "Ave", 67890, 20};
    Record c{"Sidorov S S", 3, "Blvd", 11111, 30};

    EXPECT_TRUE(ht.insert(a));
    EXPECT_TRUE(ht.insert(b));
    EXPECT_TRUE(ht.insert(c));

    size_t idx; int steps;
    EXPECT_TRUE(ht.search(b.fio, b.applicationNumber, idx, steps));
    EXPECT_EQ(ht.getOriginalLine(idx), b.originalLine);

    EXPECT_TRUE(ht.remove(b));
    EXPECT_FALSE(ht.search(b.fio, b.applicationNumber, idx, steps));

    ht.clear();
    EXPECT_FALSE(ht.search(a.fio, a.applicationNumber, idx, steps));

    ht.insert(a);
    ht.saveToFile("ht_test.txt");
    std::ifstream f("ht_test.txt");
    std::string firstLine; std::getline(f, firstLine);

    EXPECT_NE(firstLine.find("Idx"), std::string::npos);
}

TEST(HashTable, AutomaticExpansion) {
    // start with minimal size to force growth
    HashTable small(2);
    for (int i = 0; i < 10; ++i) {
        Record r{"Name" + std::to_string(i), i, "St", 100 + i, i};
        EXPECT_TRUE(small.insert(r));
    }


    // capture printed table to determine current capacity
    std::ostringstream oss; small.print(oss);
    std::string dump = oss.str();
    size_t lines = std::count(dump.begin(), dump.end(), '\n');
    // subtract header line to get number of buckets
    size_t buckets = lines > 0 ? lines - 1 : 0;
  

    EXPECT_GT(buckets, 2);

    size_t idx; int steps;
    EXPECT_TRUE(small.search("Name9", 9, idx, steps));
}
