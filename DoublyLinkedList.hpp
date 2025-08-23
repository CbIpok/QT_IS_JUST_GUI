#pragma once
#include <iostream>

class DoublyLinkedList {
public:
    DoublyLinkedList();
    DoublyLinkedList(const DoublyLinkedList& other);            // deep copy
    DoublyLinkedList(DoublyLinkedList&& other) noexcept;         // move
    DoublyLinkedList& operator=(DoublyLinkedList other) noexcept;// copy-swap
    ~DoublyLinkedList();

    void clear();

    // вставки
    void push_front(int value);
    void push_back(int value);

    // удаления
    void remove_all(int value);
    void remove_before_value(int value); // удалить каждый узел, стоящий ПЕРЕД каждым value

    // поиск/прочее
    bool contains(int value) const;
    int  length() const;
    void reverse();

    // печать
    void print(std::ostream& os = std::cout) const;

    // построение новой списка на основе "наибольшего простого меньше элемента"
    DoublyLinkedList map_to_prev_prime() const;

private:
    struct Node {
        int   data;
        Node* prev;
        Node* next;
        Node(int d) : data(d), prev(nullptr), next(nullptr) {}
    };

    Node* head_;
    Node* tail_;
    int   size_;

    static bool is_prime(int n);
    static int  largest_prime_less_than(int n);

    void swap(DoublyLinkedList& other) noexcept;
};
