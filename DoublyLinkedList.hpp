#pragma once
#include <iostream>
#include <cstddef>

// Generic doubly-linked list used for storing records in data structures.
// The list keeps elements contiguous via indices. Removing an element from the
// middle moves the last element into its place.

template <typename T>
class DoublyLinkedList {
public:
    DoublyLinkedList();
    DoublyLinkedList(const DoublyLinkedList& other);
    DoublyLinkedList(DoublyLinkedList&& other) noexcept;
    DoublyLinkedList& operator=(DoublyLinkedList other) noexcept;
    ~DoublyLinkedList();

    void clear();

    void push_front(const T& value);
    void push_back(const T& value);
    // push_back returning index (used by integrator)
    size_t push_back_index(const T& value);

    void remove_all(const T& value);
    void remove_before_value(const T& value);

    bool contains(const T& value) const;
    int  length() const;
    void reverse();

    void print(std::ostream& os = std::cout) const;

    // Access by index
    T& at(size_t index);
    const T& at(size_t index) const;

    // Remove by index and move last element into removed spot.
    // 'movedValue' receives the element moved from the end (if any), and
    // 'fromIndex' receives its previous index. If the removed element was the
    // last one, 'fromIndex' equals 'index'.
    bool remove_at(size_t index, T& movedValue, size_t& fromIndex);

private:
    struct Node {
        T     data;
        Node* prev;
        Node* next;
        Node(const T& d) : data(d), prev(nullptr), next(nullptr) {}
    };

    Node* head_;
    Node* tail_;
    int   size_;

    void swap(DoublyLinkedList& other) noexcept;
    Node* node_at(size_t index) const;
};

// ---- Implementation ----

template <typename T>
DoublyLinkedList<T>::DoublyLinkedList() : head_(nullptr), tail_(nullptr), size_(0) {}

template <typename T>
DoublyLinkedList<T>::DoublyLinkedList(const DoublyLinkedList& other) : DoublyLinkedList() {
    for (Node* cur = other.head_; cur; cur = cur->next) push_back(cur->data);
}

template <typename T>
DoublyLinkedList<T>::DoublyLinkedList(DoublyLinkedList&& other) noexcept
    : head_(other.head_), tail_(other.tail_), size_(other.size_) {
    other.head_ = other.tail_ = nullptr;
    other.size_ = 0;
}

template <typename T>
DoublyLinkedList<T>& DoublyLinkedList<T>::operator=(DoublyLinkedList other) noexcept {
    swap(other);
    return *this;
}

template <typename T>
DoublyLinkedList<T>::~DoublyLinkedList() { clear(); }

template <typename T>
void DoublyLinkedList<T>::swap(DoublyLinkedList& other) noexcept {
    std::swap(head_, other.head_);
    std::swap(tail_, other.tail_);
    std::swap(size_, other.size_);
}

template <typename T>
void DoublyLinkedList<T>::clear() {
    Node* cur = head_;
    while (cur) {
        Node* nxt = cur->next;
        delete cur;
        cur = nxt;
    }
    head_ = tail_ = nullptr;
    size_ = 0;
}

template <typename T>
void DoublyLinkedList<T>::push_front(const T& value) {
    Node* n = new Node(value);
    n->next = head_;
    if (head_) head_->prev = n;
    head_ = n;
    if (!tail_) tail_ = n;
    ++size_;
}

template <typename T>
void DoublyLinkedList<T>::push_back(const T& value) {
    Node* n = new Node(value);
    n->prev = tail_;
    if (tail_) tail_->next = n;
    tail_ = n;
    if (!head_) head_ = n;
    ++size_;
}

template <typename T>
size_t DoublyLinkedList<T>::push_back_index(const T& value) {
    push_back(value);
    return static_cast<size_t>(size_ - 1);
}

template <typename T>
void DoublyLinkedList<T>::remove_all(const T& value) {
    Node* cur = head_;
    while (cur) {
        if (cur->data == value) {
            Node* del = cur;
            Node* nxt = cur->next;
            if (del->prev) del->prev->next = del->next; else head_ = del->next;
            if (del->next) del->next->prev = del->prev; else tail_ = del->prev;
            delete del;
            cur = nxt;
            --size_;
        } else {
            cur = cur->next;
        }
    }
}

template <typename T>
void DoublyLinkedList<T>::remove_before_value(const T& value) {
    Node* cur = head_;
    while (cur && cur->next) {
        if (cur->next->data == value) {
            Node* del = cur;
            Node* nxt_after_del = cur->next;
            if (del->prev) {
                del->prev->next = del->next;
                del->next->prev = del->prev;
            } else {
                head_ = del->next;
                head_->prev = nullptr;
            }
            delete del;
            --size_;
            cur = nxt_after_del;
        } else {
            cur = cur->next;
        }
    }
}

template <typename T>
bool DoublyLinkedList<T>::contains(const T& value) const {
    for (Node* cur = head_; cur; cur = cur->next)
        if (cur->data == value) return true;
    return false;
}

template <typename T>
int DoublyLinkedList<T>::length() const { return size_; }

template <typename T>
void DoublyLinkedList<T>::reverse() {
    Node* cur = head_;
    Node* tmp = nullptr;
    while (cur) {
        tmp = cur->prev;
        cur->prev = cur->next;
        cur->next = tmp;
        cur = cur->prev;
    }
    if (tmp) {
        tail_ = head_;
        head_ = tmp->prev;
    }
}

template <typename T>
void DoublyLinkedList<T>::print(std::ostream& os) const {
    for (Node* cur = head_; cur; cur = cur->next) os << cur->data << ' ';
    os << std::endl;
}

template <typename T>
typename DoublyLinkedList<T>::Node* DoublyLinkedList<T>::node_at(size_t index) const {
    Node* cur = head_;
    for (size_t i = 0; cur && i < index; ++i) cur = cur->next;
    return cur;
}

template <typename T>
T& DoublyLinkedList<T>::at(size_t index) {
    return node_at(index)->data;
}

template <typename T>
const T& DoublyLinkedList<T>::at(size_t index) const {
    return node_at(index)->data;
}

template <typename T>
bool DoublyLinkedList<T>::remove_at(size_t index, T& movedValue, size_t& fromIndex) {
    if (index >= static_cast<size_t>(size_)) return false;
    Node* target = node_at(index);
    Node* last = tail_;
    fromIndex = static_cast<size_t>(size_ - 1);
    if (target != last) {
        target->data = last->data;
        movedValue = last->data;
    } else {
        movedValue = last->data;
        fromIndex = index;
    }
    if (last->prev) last->prev->next = nullptr; else head_ = nullptr;
    tail_ = last->prev;
    delete last;
    --size_;
    return true;
}

