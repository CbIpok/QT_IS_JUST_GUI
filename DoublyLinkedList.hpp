#pragma once

#include <cstddef>
#include <iostream>
#include <utility>

// Generic doubly linked list without using standard containers
template <typename T>
class DoublyLinkedList {
private:
    struct Node {
        T data;
        Node* prev;
        Node* next;
        Node(const T& d) : data(d), prev(nullptr), next(nullptr) {}
    };

    Node*  head_;
    Node*  tail_;
    size_t size_;

public:
    class Iterator {
        Node* node_;
    public:
        Iterator(Node* n = nullptr) : node_(n) {}
        T& operator*() const { return node_->data; }
        T* operator->() const { return &node_->data; }
        Iterator& operator++() { if (node_) node_ = node_->next; return *this; }
        bool operator!=(const Iterator& other) const { return node_ != other.node_; }
    };

    class ConstIterator {
        const Node* node_;
    public:
        ConstIterator(const Node* n = nullptr) : node_(n) {}
        const T& operator*() const { return node_->data; }
        const T* operator->() const { return &node_->data; }
        ConstIterator& operator++() { if (node_) node_ = node_->next; return *this; }
        bool operator!=(const ConstIterator& other) const { return node_ != other.node_; }
    };

    DoublyLinkedList() : head_(nullptr), tail_(nullptr), size_(0) {}

    DoublyLinkedList(const DoublyLinkedList& other) : DoublyLinkedList() {
        for (Node* cur = other.head_; cur; cur = cur->next) push_back(cur->data);
    }

    DoublyLinkedList(DoublyLinkedList&& other) noexcept
        : head_(other.head_), tail_(other.tail_), size_(other.size_) {
        other.head_ = other.tail_ = nullptr;
        other.size_ = 0;
    }

    DoublyLinkedList& operator=(DoublyLinkedList other) noexcept {
        swap(other);
        return *this;
    }

    ~DoublyLinkedList() { clear(); }

    void swap(DoublyLinkedList& other) noexcept {
        std::swap(head_, other.head_);
        std::swap(tail_, other.tail_);
        std::swap(size_, other.size_);
    }

    void clear() {
        Node* cur = head_;
        while (cur) {
            Node* nxt = cur->next;
            delete cur;
            cur = nxt;
        }
        head_ = tail_ = nullptr;
        size_ = 0;
    }

    void push_front(const T& value) {
        Node* n = new Node(value);
        n->next = head_;
        if (head_) head_->prev = n;
        head_ = n;
        if (!tail_) tail_ = n;
        ++size_;
    }

    void push_back(const T& value) {
        Node* n = new Node(value);
        n->prev = tail_;
        if (tail_) tail_->next = n;
        tail_ = n;
        if (!head_) head_ = n;
        ++size_;
    }

    void remove_all(const T& value) {
        Node* cur = head_;
        while (cur) {
            if (cur->data == value) {
                Node* del = cur;
                Node* nxt = cur->next;
                if (del->prev) del->prev->next = del->next;
                else head_ = del->next;
                if (del->next) del->next->prev = del->prev;
                else tail_ = del->prev;
                delete del;
                cur = nxt;
                --size_;
            } else {
                cur = cur->next;
            }
        }
    }

    void remove_before_value(const T& value) {
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

    bool contains(const T& value) const {
        for (Node* cur = head_; cur; cur = cur->next)
            if (cur->data == value) return true;
        return false;
    }

    size_t size() const { return size_; }

    void reverse() {
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

    void print(std::ostream& os = std::cout) const {
        for (Node* cur = head_; cur; cur = cur->next)
            os << cur->data << ' ';
        os << std::endl;
    }

    T& operator[](size_t index) {
        Node* cur = head_;
        for (size_t i = 0; cur && i < index; ++i) cur = cur->next;
        return cur->data;
    }

    const T& operator[](size_t index) const {
        const Node* cur = head_;
        for (size_t i = 0; cur && i < index; ++i) cur = cur->next;
        return cur->data;
    }

    Iterator begin() { return Iterator(head_); }
    Iterator end() { return Iterator(nullptr); }
    ConstIterator begin() const { return ConstIterator(head_); }
    ConstIterator end() const { return ConstIterator(nullptr); }
};

