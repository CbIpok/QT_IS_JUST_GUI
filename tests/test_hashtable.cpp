#include <gtest/gtest.h>
#include <algorithm>
#include <fstream>
#include <sstream>
#include "../hashtable.hpp"
#include "../record_utils.hpp"

TEST(HashTable, BasicOperations) {
    HashTable ht(4);
    Record a{"LIC001", u8"Иванов Иван Иванович", u8"Lada", u8"Улица Ленина 10", 150000, "12 Mar 2024", 10};
    Record b{"LIC002", u8"Петров Пётр Петрович", u8"Toyota", u8"Улица Мира 15", 200500, "15 Mar 2024", 20};
    Record c{"LIC003", u8"Сидоров Сергей Сергеевич", u8"Audi", u8"Проспект Победы 5", 180000, "18 Mar 2024", 30};

    EXPECT_TRUE(ht.insert(a));
    EXPECT_TRUE(ht.insert(b));
    EXPECT_TRUE(ht.insert(c));

    size_t idx; int steps;
    EXPECT_TRUE(ht.search(b.licenseNumber, idx, steps));
    EXPECT_EQ(ht.getOriginalLine(idx), b.originalLine);

    EXPECT_TRUE(ht.remove(b));
    EXPECT_FALSE(ht.search(b.licenseNumber, idx, steps));

    ht.clear();
    EXPECT_FALSE(ht.search(a.licenseNumber, idx, steps));

    ht.insert(a);
    ht.saveToFile("ht_test.txt");
    std::ifstream f("ht_test.txt");
    std::string firstLine; std::getline(f, firstLine);

    EXPECT_NE(firstLine.find("Idx"), std::string::npos);

    ht.saveReport("ht_report.csv");
    std::ifstream report("ht_report.csv");
    std::string reportLine;
    std::getline(report, reportLine);
    EXPECT_NE(reportLine.find(a.licenseNumber), std::string::npos);
}

TEST(HashTable, AutomaticExpansion) {
    // start with minimal size to force growth
    HashTable small(2);
    for (int i = 0; i < 10; ++i) {
        Record r{
            "LIC" + std::to_string(i),
            u8"Иванов Иван Иванович",
            u8"Lada",
            u8"Улица Тестовая 1",
            100000 + i,
            "01 Jan 2024",
            i
        };
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
    EXPECT_TRUE(small.search("LIC9", idx, steps));
}

