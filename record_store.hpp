#pragma once
#include <vector>
#include <cstddef>
#include <stdexcept>

// Indexed record storage built on a doubly linked list with O(1) index access
// and O(1) removal via swap-with-last semantics.
template <typename T>
class RecordStore {
public:
    struct Node {
        T data;
        Node* prev{nullptr};
        Node* next{nullptr};
        explicit Node(const T& d) : data(d) {}
    };

    struct RemoveResult {
        bool removed{false};
        // If a swap occurred, this is the element that moved from last to idx
        bool moved{false};
        std::size_t moved_from{static_cast<std::size_t>(-1)};
        std::size_t moved_to{static_cast<std::size_t>(-1)};
        T moved_element{}; // valid only if moved==true
    };

    RecordStore() = default;
    ~RecordStore() { clear(); }

    std::size_t size() const { return index_.size(); }
    bool empty() const { return index_.empty(); }

    void clear() {
        Node* cur = head_;
        while (cur) {
            Node* nxt = cur->next;
            delete cur;
            cur = nxt;
        }
        head_ = tail_ = nullptr;
        index_.clear();
    }

    // Append element, return its index
    std::size_t add(const T& value) {
        Node* n = new Node(value);
        if (!tail_) {
            head_ = tail_ = n;
        } else {
            tail_->next = n;
            n->prev = tail_;
            tail_ = n;
        }
        index_.push_back(n);
        return index_.size() - 1;
    }

    // Access by index
    T& get(std::size_t i) {
        if (i >= index_.size()) throw std::out_of_range("RecordStore index out of range");
        return index_[i]->data;
    }
    const T& get(std::size_t i) const {
        if (i >= index_.size()) throw std::out_of_range("RecordStore index out of range");
        return index_[i]->data;
    }

    // Remove element at index. If not last, swaps last into index i and reports it.
    RemoveResult remove_at(std::size_t i) {
        RemoveResult res;
        if (i >= index_.size()) return res;
        std::size_t last = index_.size() - 1;
        Node* to_delete = index_[last];
        if (i != last) {
            // Copy data of last into node at i; keep node pointers stable.
            res.moved = true;
            res.moved_from = last;
            res.moved_to = i;
            res.moved_element = to_delete->data; // snapshot for external index updates
            index_[i]->data = to_delete->data;
        }
        // Remove tail node
        // unlink from list
        if (to_delete->prev) to_delete->prev->next = to_delete->next;
        if (to_delete->next) to_delete->next->prev = to_delete->prev;
        if (tail_ == to_delete) tail_ = to_delete->prev;
        if (head_ == to_delete) head_ = to_delete->next;
        delete to_delete;
        index_.pop_back();
        res.removed = true;
        return res;
    }

private:
    Node* head_{nullptr};
    Node* tail_{nullptr};
    std::vector<Node*> index_;
};
