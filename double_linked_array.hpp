#ifndef DOUBLE_LINKED_ARRAY_HPP
#define DOUBLE_LINKED_ARRAY_HPP

#include <cstddef>
#include <vector>
#include <iostream>

// Simple doubly linked list implementation backed by a fixed-size array.
// Each element keeps indexes of the previous and next element which allows
// traversal in both directions while storing data in a contiguous block.
class DoubleLinkedArray {
public:
    explicit DoubleLinkedArray(std::size_t capacity);
    ~DoubleLinkedArray();

    // Add value to the end of the list. Returns index of the new node or -1
    // if the structure is full.
    int  pushBack(int value);
    // Insert value after the node at index. Returns false on error.
    bool insertAfter(int index, int value);
    // Remove the node at index. Returns false if index is invalid.
    bool remove(int index);
    // Obtain the value stored at index. Undefined for invalid index.
    int  get(int index) const;
    // Remove all elements and reset to initial state.
    void clear();
    // Print values from head to tail.
    void print(std::ostream& out) const;
    std::size_t size() const { return m_count; }

private:
    struct Node {
        int value;
        int prev;
        int next;
        bool used;
    };

    std::vector<Node> nodes;
    int head;
    int tail;
    int freeHead;
    std::size_t m_count;

    void initFreeList();
    int  allocateNode();
    void freeNode(int index);
};

#endif // DOUBLE_LINKED_ARRAY_HPP

