#include "doubly_linked_array.hpp"
#include <cassert>
#include <sstream>

int main() {
    DoublyLinkedArray<int> list;
    int a = list.push_back(1);
    int b = list.push_back(2);
    int c = list.push_back(3);

    auto nodeA = list.at(a);
    auto nodeB = list.at(b);
    auto nodeC = list.at(c);
    assert(nodeA && nodeB && nodeC);
    assert(nodeA->next == b && nodeB->prev == a);
    assert(nodeB->next == c && nodeC->prev == b);

    // Remove middle node
    assert(list.remove(b));
    assert(list.at(b) == nullptr);
    nodeA = list.at(a);
    nodeC = list.at(c);
    assert(nodeA->next == c && nodeC->prev == a);

    // Print to verify order
    std::ostringstream oss;
    list.print(oss);
    assert(oss.str() == "1 <-> 3");

    // Remove remaining nodes and reuse storage
    assert(list.remove(a));
    assert(list.remove(c));
    int d = list.push_back(4);
    auto nodeD = list.at(d);
    assert(nodeD && nodeD->value == 4);
    return 0;
}
