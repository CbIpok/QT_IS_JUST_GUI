#include "avl_tree_idx.hpp"

AVLTreeIndex::AVLTreeIndex() = default;
AVLTreeIndex::~AVLTreeIndex() { clear(); }

int AVLTreeIndex::height(Node* n) { return n ? n->height : 0; }
void AVLTreeIndex::update_height(Node* n) { if (n) n->height = 1 + std::max(height(n->left), height(n->right)); }
int AVLTreeIndex::balance(Node* n) { return n ? height(n->left) - height(n->right) : 0; }

AVLTreeIndex::Node* AVLTreeIndex::rotate_right(Node* y) {
    Node* x = y->left; Node* t2 = x->right; x->right = y; y->left = t2; update_height(y); update_height(x); return x;
}
AVLTreeIndex::Node* AVLTreeIndex::rotate_left(Node* x) {
    Node* y = x->right; Node* t2 = y->left; y->left = x; x->right = t2; update_height(x); update_height(y); return y;
}
AVLTreeIndex::Node* AVLTreeIndex::balance_node(Node* n) {
    update_height(n);
    int b = balance(n);
    if (b > 1 && balance(n->left) >= 0) return rotate_right(n);
    if (b > 1 && balance(n->left) < 0) { n->left = rotate_left(n->left); return rotate_right(n);} 
    if (b < -1 && balance(n->right) <= 0) return rotate_left(n);
    if (b < -1 && balance(n->right) > 0) { n->right = rotate_right(n->right); return rotate_left(n);} 
    return n;
}

AVLTreeIndex::Node* AVLTreeIndex::min_node(Node* n) { while (n && n->left) n = n->left; return n; }

AVLTreeIndex::Node* AVLTreeIndex::insert_node(Node* n, const OrderKey& key, std::size_t index) {
    if (!n) { auto* x = new Node(); x->key = key; x->indices.push_back(index); return x; }
    int cmp = order_key_compare(key, n->key);
    if (cmp < 0) n->left = insert_node(n->left, key, index);
    else if (cmp > 0) n->right = insert_node(n->right, key, index);
    else { n->indices.push_back(index); return n; }
    return balance_node(n);
}

AVLTreeIndex::Node* AVLTreeIndex::remove_node(Node* n, const OrderKey& key, bool& removed, std::vector<std::size_t>* outIndices) {
    if (!n) return nullptr;
    int cmp = order_key_compare(key, n->key);
    if (cmp < 0) n->left = remove_node(n->left, key, removed, outIndices);
    else if (cmp > 0) n->right = remove_node(n->right, key, removed, outIndices);
    else {
        removed = true;
        if (outIndices) *outIndices = n->indices;
        if (!n->left || !n->right) {
            Node* tmp = n->left ? n->left : n->right;
            delete n; return tmp;
        } else {
            Node* m = min_node(n->right);
            n->key = m->key; n->indices = m->indices;
            bool dummy=false; std::vector<std::size_t> ignored; n->right = remove_node(n->right, m->key, dummy, &ignored);
        }
    }
    return balance_node(n);
}

AVLTreeIndex::Node* AVLTreeIndex::search_node(Node* n, const OrderKey& key) {
    while (n) {
        int cmp = order_key_compare(key, n->key);
        if (cmp == 0) return n;
        n = (cmp < 0) ? n->left : n->right;
    }
    return nullptr;
}

void AVLTreeIndex::free_node(Node* n) { if (!n) return; free_node(n->left); free_node(n->right); delete n; }

void AVLTreeIndex::inorder_collect(Node* n, const std::string& license, std::vector<OrderKey>& keys) {
    if (!n) return; inorder_collect(n->left, license, keys); if (n->key.licenseNumber == license) keys.push_back(n->key); inorder_collect(n->right, license, keys);
}

void AVLTreeIndex::insert(const OrderKey& key, std::size_t index) { root_ = insert_node(root_, key, index); }

bool AVLTreeIndex::remove_key(const OrderKey& key, std::vector<std::size_t>& removedIndices) {
    bool removed=false; root_ = remove_node(root_, key, removed, &removedIndices); return removed;
}

bool AVLTreeIndex::remove_single(const OrderKey& key, std::size_t index) {
    Node* n = search_node(root_, key);
    if (!n) return false;
    for (std::size_t i=0;i<n->indices.size();++i) {
        if (n->indices[i] == index) {
            // swap-with-last removal from vector
            std::size_t last = n->indices.size()-1;
            if (i != last) n->indices[i] = n->indices[last];
            n->indices.pop_back();
            if (n->indices.empty()) {
                std::vector<std::size_t> ignored; remove_key(key, ignored);
            }
            return true;
        }
    }
    return false;
}

bool AVLTreeIndex::find_indices(const OrderKey& key, std::vector<std::size_t>& out) const {
    Node* n = search_node(const_cast<Node*>(root_), key);
    if (!n) return false; out = n->indices; return true;
}

std::vector<OrderKey> AVLTreeIndex::collect_keys_with_license(const std::string& license) const {
    std::vector<OrderKey> keys; inorder_collect(const_cast<Node*>(root_), license, keys); return keys;
}

bool AVLTreeIndex::update_index_for_key(const OrderKey& key, std::size_t oldIndex, std::size_t newIndex) {
    Node* n = search_node(root_, key);
    if (!n) return false;
    for (auto& idx : n->indices) {
        if (idx == oldIndex) { idx = newIndex; return true; }
    }
    return false;
}

void AVLTreeIndex::clear() { free_node(root_); root_ = nullptr; }

