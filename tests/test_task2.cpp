
// Google Test for merge sort and search routines from task 2.2
#include <gtest/gtest.h>
#include <vector>
#include "../merge_sort.hpp"
#include "../modnaminecraft.hpp"

TEST(Task2, SortAndSearch) {

    std::vector<Record> recs = {
        {"Ivanov","I","I","Street1",111111,3,0},
        {"Petrov","P","P","Street2",222222,1,1},
        {"Sidorov","S","S","Street3",333333,2,2},
        {"Fedorov","F","F","Street4",444444,1,3}
    };
    mergeSort(recs.data(), 0, static_cast<int>(recs.size()) - 1);

    std::vector<int> keys;
    for (const auto &r : recs) keys.push_back(r.applicationNumber);
    EXPECT_EQ(keys, (std::vector<int>{1, 1, 2, 3}));

    auto bin = binarySearch(keys, 1);
    EXPECT_EQ(bin.first, 0);
    EXPECT_GT(bin.second, 0);

    auto lin = linearSearch(keys, 2);
    EXPECT_EQ(lin.first, (std::vector<int>{2}));
    EXPECT_EQ(lin.second, static_cast<int>(keys.size()));
}
