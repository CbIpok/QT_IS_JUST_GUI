#pragma once
#include <string>
#include <vector>
#include <cstddef>

struct OrderKey {
    std::string licenseNumber;
    std::string address;
};

inline int order_key_compare(const OrderKey& a, const OrderKey& b) {
    if (a.licenseNumber < b.licenseNumber) return -1;
    if (a.licenseNumber > b.licenseNumber) return 1;
    if (a.address < b.address) return -1;
    if (a.address > b.address) return 1;
    return 0;
}

// AVL tree storing indices to an external OrderRecord storage.
class AVLTreeIndex {
public:
    AVLTreeIndex();
    ~AVLTreeIndex();

    void insert(const OrderKey& key, std::size_t index);
    // Remove the whole key node, returning all indices attached to it.
    bool remove_key(const OrderKey& key, std::vector<std::size_t>& removedIndices);
    // Remove a single index from a key, deleting key if last.
    bool remove_single(const OrderKey& key, std::size_t index);

    bool find_indices(const OrderKey& key, std::vector<std::size_t>& out) const;
    // Collect all keys with given licenseNumber
    std::vector<OrderKey> collect_keys_with_license(const std::string& license) const;

    // Update moved index for a given key (old->new)
    bool update_index_for_key(const OrderKey& key, std::size_t oldIndex, std::size_t newIndex);

    void clear();

private:
    struct Node {
        OrderKey key;
        int height{1};
        Node* left{nullptr};
        Node* right{nullptr};
        std::vector<std::size_t> indices; // duplicates allowed
    };
    Node* root_{nullptr};

    static int height(Node* n);
    static void update_height(Node* n);
    static int balance(Node* n);
    static Node* rotate_left(Node* x);
    static Node* rotate_right(Node* y);
    static Node* balance_node(Node* n);

    static Node* min_node(Node* n);
    static Node* insert_node(Node* n, const OrderKey& key, std::size_t index);
    static Node* remove_node(Node* n, const OrderKey& key, bool& removed, std::vector<std::size_t>* outIndices);
    static Node* search_node(Node* n, const OrderKey& key);
    static void free_node(Node* n);
    static void inorder_collect(Node* n, const std::string& license, std::vector<OrderKey>& keys);
};

