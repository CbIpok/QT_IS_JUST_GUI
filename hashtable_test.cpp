#include "hashtable.hpp"
#include <cassert>
#include <sstream>

int main() {
    HashTable table(8);
    Record r1{"Alice", 1, "Street1", 1111, 10};
    Record r2{"Bob", 2, "Street2", 2222, 20};

    // Insert and search
    assert(table.insert(r1));
    assert(table.insert(r2));
    assert(!table.insert(r1)); // duplicate should fail

    size_t idx; int steps;
    assert(table.search("Alice", 1, idx, steps));
    assert(table.getOriginalLine(idx) == 10);

    // Remove and ensure not found
    assert(table.remove(r1));
    assert(!table.search("Alice", 1, idx, steps));
    assert(!table.remove(r1));

    // Clear and verify
    table.clear();
    assert(!table.search("Bob", 2, idx, steps));
    return 0;
}
