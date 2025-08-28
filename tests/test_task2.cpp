
// Google Test for merge sort and search routines from task 2.2
#include <gtest/gtest.h>
#include <vector>
#include "../merge_sort.hpp"
#include "../modnaminecraft.hpp"

TEST(Task2, SortAndSearch) {

    std::vector<OrderRecord> recs = {
        {"TK-25-111111-2023", "Ул. Лесная, дом 10", 300, "02 jan 2025", 0},
        {"TK-25-222222-2024", "Ул. Ленина, дом 25", 500, "07 jan 2025", 1},
        {"TK-25-333333-2022", "Ул. Победы, дом 50", 250, "12 jan 2025", 2},
        {"TK-25-444444-2025", "Ул. Мира, дом 5", 450, "17 jan 2025", 3}
    };
    mergeSort(recs.data(), 0, static_cast<int>(recs.size()) - 1);

    std::vector<int> keys;
    for (const auto &r : recs) keys.push_back(r.cost);
    EXPECT_EQ(keys, (std::vector<int>{250, 300, 450, 500}));

    auto bin = binarySearch(keys, 300);
    EXPECT_EQ(bin.first, 1);
    EXPECT_GT(bin.second, 0);

    auto lin = linearSearch(keys, 450);
    EXPECT_EQ(lin.first, (std::vector<int>{2}));
    EXPECT_EQ(lin.second, static_cast<int>(keys.size()));
}
