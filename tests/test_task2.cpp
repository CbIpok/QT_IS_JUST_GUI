// Google Test for merge sort and search routines from task 2.2
#include <gtest/gtest.h>
#include <vector>
#include "../merge_sort.hpp"
#include "../modnaminecraft.hpp"
#include "../record_utils.hpp"

TEST(Task2, SortAndSearch) {

    std::vector<Record> recs = {
        {"LIC001", u8"Иванов Иван Иванович", u8"Lada", u8"Улица Ленина 10", 150000, "01 Jan 2024", 0},
        {"LIC002", u8"Петров Пётр Петрович", u8"Toyota", u8"Улица Мира 5", 120500, "02 Jan 2024", 1},
        {"LIC003", u8"Сидоров Сергей Сергеевич", u8"Audi", u8"Проспект Победы 7", 180000, "03 Jan 2024", 2},
        {"LIC004", u8"Фёдоров Фёдор Фёдорович", u8"Bmw", u8"Улица Гагарина 12", 120500, "04 Jan 2024", 3}
    };
    mergeSort(recs.data(), 0, static_cast<int>(recs.size()) - 1);

    std::vector<int> keys;
    for (const auto &r : recs) keys.push_back(r.costKopecks);
    EXPECT_EQ(keys, (std::vector<int>{120500, 120500, 150000, 180000}));

    auto bin = binarySearch(keys, 120500);
    EXPECT_EQ(bin.first, 0);
    EXPECT_GT(bin.second, 0);

    auto lin = linearSearch(keys, 150000);
    EXPECT_EQ(lin.first, (std::vector<int>{2}));
    EXPECT_EQ(lin.second, static_cast<int>(keys.size()));
}

