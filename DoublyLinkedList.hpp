#pragma once

#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <type_traits>
#include <utility>


namespace detail {
template <typename U>
class is_stream_insertable {
private:
    template <typename V>
    static auto test(int) -> decltype(std::declval<std::ostream&>() << std::declval<const V&>(), std::true_type{});
    template <typename>
    static auto test(...) -> std::false_type;
public:
    static constexpr bool value = decltype(test<U>(0))::value;
};

template <typename U>
constexpr bool is_stream_insertable_v = is_stream_insertable<U>::value;
}

// Doubly-linked list that supports storing arbitrary values while
// keeping index-based access helpers needed by the integrator layer.
// Existing behaviour for int-based operations remains available.
template <typename T>
class DoublyLinkedList {
public:
    struct SwapRemoveResult {
        T      removedValue{};
        bool   swapped = false;
        size_t swappedFromIndex = 0; // index the replacement element used to occupy
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

    void clear() {
        Node* cur = head_;
        while (cur) {
            Node* next = cur->next;
            delete cur;
            cur = next;
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

    void push_front(T&& value) {
        Node* n = new Node(std::move(value));
        n->next = head_;
        if (head_) head_->prev = n;
        head_ = n;
        if (!tail_) tail_ = n;
        ++size_;
    }

    size_t push_back(const T& value) {
        Node* n = new Node(value);
        n->prev = tail_;
        if (tail_) tail_->next = n;
        tail_ = n;
        if (!head_) head_ = n;
        ++size_;
        return size_ - 1;
    }

    size_t push_back(T&& value) {
        Node* n = new Node(std::move(value));
        n->prev = tail_;
        if (tail_) tail_->next = n;
        tail_ = n;
        if (!head_) head_ = n;
        ++size_;
        return size_ - 1;
    }

    void remove_all(const T& value) {
        Node* cur = head_;
        while (cur) {
            if (cur->data == value) {
                Node* toDelete = cur;
                Node* next = cur->next;
                unlink_node(toDelete);
                delete toDelete;
                --size_;
                cur = next;
            }
            else {
                cur = cur->next;
            }
        }
    }

    void remove_before_value(const T& value) {
        Node* cur = head_;
        while (cur && cur->next) {
            if (cur->next->data == value) {
                Node* toDelete = cur;
                Node* resume = cur->next;
                unlink_node(toDelete);
                delete toDelete;
                --size_;
                cur = resume;
            }
            else {
                cur = cur->next;
            }
        }
    }

    bool contains(const T& value) const {
        for (Node* cur = head_; cur; cur = cur->next) {
            if (cur->data == value) return true;
        }
        return false;
    }

    int length() const { return static_cast<int>(size_); }
    size_t size() const { return size_; }
    bool empty() const { return size_ == 0; }

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
        if constexpr (detail::is_stream_insertable_v<T>) {
            for (Node* cur = head_; cur; cur = cur->next) os << cur->data << ' ';
        }
        else {
            for (Node* cur = head_; cur; cur = cur->next) os << "[node] " ;
        }
        os << '\n';
    }

    template <typename U = T>
    std::enable_if_t<std::is_integral_v<U>, DoublyLinkedList<int>> map_to_prev_prime() const {
        DoublyLinkedList<int> out;
        for (Node* cur = head_; cur; cur = cur->next)
            out.push_back(largest_prime_less_than(static_cast<int>(cur->data)));
        return out;
    }

    T& at(size_t index) {
        Node* n = node_at(index);
        if (!n) throw std::out_of_range("index out of range");
        return n->data;
    }

    const T& at(size_t index) const {
        Node* n = node_at(index);
        if (!n) throw std::out_of_range("index out of range");
        return n->data;
    }

    bool remove_by_index(size_t index, SwapRemoveResult& result) {
        if (index >= size_) return false;
        Node* target = node_at(index);
        Node* last = tail_;
        size_t lastIndex = size_ - 1;
        result.removedValue = target->data;
        if (target == last) {
            unlink_node(last);
            delete last;
            --size_;
            result.swapped = false;
            result.swappedFromIndex = lastIndex;
        }
        else {
            target->data = std::move(last->data);
            unlink_node(last);
            delete last;
            --size_;
            result.swapped = true;
            result.swappedFromIndex = lastIndex;
        }
        if (size_ == 0) {
            head_ = tail_ = nullptr;
        }
        return true;
    }

    template <typename Func>
    void for_each(Func&& f) const {
        Node* cur = head_;
        size_t idx = 0;
        while (cur) {
            f(cur->data, idx);
            cur = cur->next;
            ++idx;
        }
    }

    void swap(DoublyLinkedList& other) noexcept {
        std::swap(head_, other.head_);
        std::swap(tail_, other.tail_);
        std::swap(size_, other.size_);
    }

private:
    struct Node {
        T      data;
        Node*  prev;
        Node*  next;
        explicit Node(const T& d) : data(d), prev(nullptr), next(nullptr) {}
        explicit Node(T&& d) : data(std::move(d)), prev(nullptr), next(nullptr) {}
    };

    Node* head_;
    Node* tail_;
    size_t size_;

    Node* node_at(size_t index) const {
        if (index >= size_) return nullptr;
        if (index <= size_ / 2) {
            Node* cur = head_;
            for (size_t i = 0; i < index; ++i) cur = cur->next;
            return cur;
        }
        else {
            Node* cur = tail_;
            for (size_t i = size_ - 1; i > index; --i) cur = cur->prev;
            return cur;
        }
    }

    void unlink_node(Node* node) {
        if (!node) return;
        if (node->prev) node->prev->next = node->next;
        else head_ = node->next;

        if (node->next) node->next->prev = node->prev;
        else tail_ = node->prev;
    }

    static bool is_prime(int n) {
        if (n <= 1) return false;
        if (n == 2) return true;
        if (n % 2 == 0) return false;
        for (int i = 3; 1LL * i * i <= n; i += 2)
            if (n % i == 0) return false;
        return true;
    }

    static int largest_prime_less_than(int n) {
        for (int i = n - 1; i >= 2; --i)
            if (is_prime(i)) return i;
        return -1;
    }
};



