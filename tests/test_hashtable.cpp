#include <cassert>
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include "../hashtable.hpp"

int main() {
    HashTable ht(4);
    Record a{"Ivanov I I", 1, "Street", 12345, 10};
    Record b{"Petrov P P", 2, "Ave", 67890, 20};
    Record c{"Sidorov S S", 3, "Blvd", 11111, 30};
    assert(ht.insert(a));
    assert(ht.insert(b));
    assert(ht.insert(c));
    size_t idx; int steps;
    assert(ht.search(b.fio, b.applicationNumber, idx, steps));
    assert(ht.getOriginalLine(idx) == b.originalLine);
    assert(ht.remove(b));
    assert(!ht.search(b.fio, b.applicationNumber, idx, steps));
    ht.clear();
    assert(!ht.search(a.fio, a.applicationNumber, idx, steps));
    ht.insert(a);
    ht.saveToFile("ht_test.txt");
    std::ifstream f("ht_test.txt");
    std::string firstLine; std::getline(f, firstLine);
    assert(firstLine.find("Idx") != std::string::npos);

    // check automatic table expansion when load factor is exceeded
    HashTable small(2); // start with minimal size to force growth
    for (int i = 0; i < 10; ++i) {
        Record r{"Name" + std::to_string(i), i, "St", 100 + i, i};
        assert(small.insert(r));
    }
    // capture printed table to determine current capacity
    std::ostringstream oss; small.print(oss);
    std::string dump = oss.str();
    size_t lines = std::count(dump.begin(), dump.end(), '\n');
    // subtract header line to get number of buckets
    size_t buckets = lines > 0 ? lines - 1 : 0;
    assert(buckets > 2); // capacity must grow beyond initial size
    // ensure one of the later records is still searchable after rehashing
    assert(small.search("Name9", 9, idx, steps));
    std::cout << "Module 2.3 tests passed\n";
    return 0;
}
