#ifndef DOUBLY_LINKED_ARRAY_HPP
#define DOUBLY_LINKED_ARRAY_HPP

#include <vector>
#include <iostream>

// A simple doubly linked list implemented on top of a vector.  Each node is
// addressed by its index in the underlying array.  The container provides
// basic operations for adding, removing and accessing nodes by index.

template <typename T>
class DoublyLinkedArray {
public:
    struct Node {
        T   value;
        int prev;
        int next;
        bool used;
        Node() : value(), prev(-1), next(-1), used(false) {}
    };

    explicit DoublyLinkedArray(size_t capacity = 0)
        : m_nodes(capacity), m_head(-1), m_tail(-1) {}

    int push_back(const T& value) {
        int idx = allocateNode();
        Node& n = m_nodes[idx];
        n.value = value;
        n.prev = m_tail;
        n.next = -1;
        if (m_tail != -1) m_nodes[m_tail].next = idx;
        m_tail = idx;
        if (m_head == -1) m_head = idx;
        return idx;
    }

    bool remove(int index) {
        if (index < 0 || index >= static_cast<int>(m_nodes.size()) || !m_nodes[index].used)
            return false;
        Node& n = m_nodes[index];
        if (n.prev != -1) m_nodes[n.prev].next = n.next; else m_head = n.next;
        if (n.next != -1) m_nodes[n.next].prev = n.prev; else m_tail = n.prev;
        n.used = false;
        n.prev = n.next = -1;
        m_free.push_back(index);
        return true;
    }

    Node* at(int index) {
        if (index < 0 || index >= static_cast<int>(m_nodes.size()) || !m_nodes[index].used)
            return nullptr;
        return &m_nodes[index];
    }

    void print(std::ostream& out) const {
        int idx = m_head;
        while (idx != -1) {
            out << m_nodes[idx].value;
            idx = m_nodes[idx].next;
            if (idx != -1) out << " <-> ";
        }
    }

private:
    int allocateNode() {
        if (!m_free.empty()) {
            int idx = m_free.back();
            m_free.pop_back();
            m_nodes[idx].used = true;
            return idx;
        }
        m_nodes.push_back(Node());
        m_nodes.back().used = true;
        return static_cast<int>(m_nodes.size()) - 1;
    }

    std::vector<Node> m_nodes;
    std::vector<int>  m_free;
    int m_head;
    int m_tail;
};

#endif // DOUBLY_LINKED_ARRAY_HPP
