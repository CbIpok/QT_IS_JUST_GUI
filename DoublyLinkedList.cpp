#include "DoublyLinkedList.hpp"
#include <utility>
#include <climits>

DoublyLinkedList::DoublyLinkedList() : head_(nullptr), tail_(nullptr), size_(0) {}

DoublyLinkedList::DoublyLinkedList(const DoublyLinkedList& other)
    : DoublyLinkedList() {
    for (Node* cur = other.head_; cur; cur = cur->next) push_back(cur->data);
}

DoublyLinkedList::DoublyLinkedList(DoublyLinkedList&& other) noexcept
    : head_(other.head_), tail_(other.tail_), size_(other.size_) {
    other.head_ = other.tail_ = nullptr;
    other.size_ = 0;
}

DoublyLinkedList& DoublyLinkedList::operator=(DoublyLinkedList other) noexcept {
    swap(other);
    return *this;
}

DoublyLinkedList::~DoublyLinkedList() { clear(); }

void DoublyLinkedList::swap(DoublyLinkedList& other) noexcept {
    std::swap(head_, other.head_);
    std::swap(tail_, other.tail_);
    std::swap(size_, other.size_);
}

void DoublyLinkedList::clear() {
    Node* cur = head_;
    while (cur) {
        Node* nxt = cur->next;
        delete cur;
        cur = nxt;
    }
    head_ = tail_ = nullptr;
    size_ = 0;
}

void DoublyLinkedList::push_front(int value) {
    Node* n = new Node(value);
    n->next = head_;
    if (head_) head_->prev = n;
    head_ = n;
    if (!tail_) tail_ = n;
    ++size_;
}

void DoublyLinkedList::push_back(int value) {
    Node* n = new Node(value);
    n->prev = tail_;
    if (tail_) tail_->next = n;
    tail_ = n;
    if (!head_) head_ = n;
    ++size_;
}

void DoublyLinkedList::remove_all(int value) {
    Node* cur = head_;
    while (cur) {
        if (cur->data == value) {
            Node* del = cur;
            Node* nxt = cur->next;

            if (del->prev) del->prev->next = del->next;
            else           head_ = del->next;

            if (del->next) del->next->prev = del->prev;
            else           tail_ = del->prev;

            delete del;
            cur = nxt;
            --size_;
        }
        else {
            cur = cur->next;
        }
    }
}

void DoublyLinkedList::remove_before_value(int value) {
    // удалить каждый узел, который непосредственно предшествует узлу со значением 'value'
    Node* cur = head_;
    while (cur && cur->next) {
        if (cur->next->data == value) {
            Node* del = cur;
            Node* nxt_after_del = cur->next;

            if (del->prev) {
                del->prev->next = del->next;
                del->next->prev = del->prev;
            }
            else {
                // удаляем голову
                head_ = del->next;
                head_->prev = nullptr;
            }
            delete del;
            --size_;
            cur = nxt_after_del; // продолжить с узла, после которого мы удалили предшественника
        }
        else {
            cur = cur->next;
        }
    }
}

bool DoublyLinkedList::contains(int value) const {
    for (Node* cur = head_; cur; cur = cur->next)
        if (cur->data == value) return true;
    return false;
}

int DoublyLinkedList::length() const { return size_; }

void DoublyLinkedList::reverse() {
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

void DoublyLinkedList::print(std::ostream& os) const {
    for (Node* cur = head_; cur; cur = cur->next) os << cur->data << ' ';
    os << std::endl;
}

bool DoublyLinkedList::is_prime(int n) {
    if (n <= 1) return false;
    if (n == 2) return true;
    if (n % 2 == 0) return false;
    for (int i = 3; 1LL * i * i <= n; i += 2)
        if (n % i == 0) return false;
    return true;
}

int DoublyLinkedList::largest_prime_less_than(int n) {
    for (int i = n - 1; i >= 2; --i)
        if (is_prime(i)) return i;
    return -1;
}

DoublyLinkedList DoublyLinkedList::map_to_prev_prime() const {
    DoublyLinkedList out;
    for (Node* cur = head_; cur; cur = cur->next)
        out.push_back(largest_prime_less_than(cur->data));
    return out;
}
