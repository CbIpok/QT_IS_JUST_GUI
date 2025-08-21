#include "double_linked_array.hpp"

DoubleLinkedArray::DoubleLinkedArray(std::size_t capacity)
    : nodes(capacity), head(-1), tail(-1), freeHead(0), m_count(0) {
    initFreeList();
}

DoubleLinkedArray::~DoubleLinkedArray() {
    // vector will clean up automatically
}

void DoubleLinkedArray::initFreeList() {
    for (std::size_t i = 0; i < nodes.size(); ++i) {
        nodes[i].used = false;
        nodes[i].next = (i + 1 < nodes.size()) ? static_cast<int>(i + 1) : -1;
    }
}

int DoubleLinkedArray::allocateNode() {
    if (freeHead == -1) return -1;
    int idx = freeHead;
    freeHead = nodes[idx].next;
    nodes[idx].used = true;
    nodes[idx].prev = nodes[idx].next = -1;
    ++m_count;
    return idx;
}

void DoubleLinkedArray::freeNode(int index) {
    nodes[index].used = false;
    nodes[index].next = freeHead;
    freeHead = index;
    --m_count;
}

int DoubleLinkedArray::pushBack(int value) {
    int idx = allocateNode();
    if (idx == -1) return -1;
    nodes[idx].value = value;
    nodes[idx].prev = tail;
    nodes[idx].next = -1;
    if (tail != -1) nodes[tail].next = idx;
    else head = idx;
    tail = idx;
    return idx;
}

bool DoubleLinkedArray::insertAfter(int index, int value) {
    if (index < 0 || index >= static_cast<int>(nodes.size()) || !nodes[index].used)
        return false;
    if (index == tail) {
        pushBack(value);
        return true;
    }
    int idx = allocateNode();
    if (idx == -1) return false;
    int nxt = nodes[index].next;
    nodes[idx].value = value;
    nodes[idx].next = nxt;
    nodes[idx].prev = index;
    nodes[index].next = idx;
    if (nxt != -1) nodes[nxt].prev = idx;
    return true;
}

bool DoubleLinkedArray::remove(int index) {
    if (index < 0 || index >= static_cast<int>(nodes.size()) || !nodes[index].used)
        return false;
    int prev = nodes[index].prev;
    int next = nodes[index].next;
    if (prev != -1) nodes[prev].next = next;
    else head = next;
    if (next != -1) nodes[next].prev = prev;
    else tail = prev;
    freeNode(index);
    return true;
}

int DoubleLinkedArray::get(int index) const {
    if (index < 0 || index >= static_cast<int>(nodes.size()) || !nodes[index].used)
        return 0;
    return nodes[index].value;
}

void DoubleLinkedArray::clear() {
    head = tail = -1;
    freeHead = 0;
    m_count = 0;
    initFreeList();
}

void DoubleLinkedArray::print(std::ostream& out) const {
    int idx = head;
    out << "[";
    bool first = true;
    while (idx != -1) {
        if (!first) out << ", ";
        out << nodes[idx].value;
        first = false;
        idx = nodes[idx].next;
    }
    out << "]";
}

