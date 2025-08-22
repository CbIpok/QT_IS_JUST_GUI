#include <cassert>
#include <sstream>
#include <iostream>
#include "../doubly_linked_array.hpp"

int main() {
    DoublyLinkedArray<int> list;
    int a = list.push_back(1);
    int b = list.push_back(2);
    int c = list.push_back(3);
    assert(list.at(a)->value == 1);
    assert(list.at(b)->prev == a);
    assert(list.at(c)->prev == b);
    assert(list.remove(b));
    assert(list.at(b) == nullptr);
    std::ostringstream oss;
    list.print(oss);
    assert(oss.str() == "1 <-> 3");
    std::cout << "Doubly linked list tests passed\n";
    return 0;
}
