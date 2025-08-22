#include <cassert>
#include <vector>
#include <iostream>
#include "merge_sort.hpp"
#include "modnaminecraft.hpp"

static void test_sort_and_search() {
    std::vector<Record> recs = {
        {"Ivanov","I","I","Street1",111111,3,0},
        {"Petrov","P","P","Street2",222222,1,1},
        {"Sidorov","S","S","Street3",333333,2,2},
        {"Fedorov","F","F","Street4",444444,1,3}
    };
    mergeSort(recs.data(), 0, static_cast<int>(recs.size()) - 1);
    std::vector<int> keys;
    for (const auto &r : recs) keys.push_back(r.applicationNumber);
    assert((keys == std::vector<int>{1,1,2,3}));
    auto bin = binarySearch(keys, 1);
    assert(bin.first == 0);
    assert(bin.second > 0);
    auto lin = linearSearch(keys, 2);
    assert((lin.first == std::vector<int>{2}));
    assert(lin.second == static_cast<int>(keys.size()));
}

int main() {
    test_sort_and_search();
    std::cout << "Module 2.2 tests passed\n";
    return 0;
}
