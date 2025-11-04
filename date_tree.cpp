#include "date_tree.hpp"

#include <utility>

namespace {

int node_height(DateTreeNode* node) {
    return node ? node->height : 0;
}

int balance_factor(DateTreeNode* node) {
    return node ? node_height(node->left) - node_height(node->right) : 0;
}

void update_height(DateTreeNode* node) {
    if (!node) {
        return;
    }
    int left = node_height(node->left);
    int right = node_height(node->right);
    node->height = (left > right ? left : right) + 1;
}

DateTreeNode* rotate_left(DateTreeNode* x) {
    DateTreeNode* y = x->right;
    DateTreeNode* T2 = y->left;
    y->left = x;
    x->right = T2;
    update_height(x);
    update_height(y);
    return y;
}

DateTreeNode* rotate_right(DateTreeNode* y) {
    DateTreeNode* x = y->left;
    DateTreeNode* T2 = x->right;
    x->right = y;
    y->left = T2;
    update_height(y);
    update_height(x);
    return x;
}

DateTreeNode* balance(DateTreeNode* node) {
    if (!node) {
        return nullptr;
    }
    update_height(node);
    int balanceValue = balance_factor(node);
    if (balanceValue > 1 && balance_factor(node->left) >= 0) {
        return rotate_right(node);
    }
    if (balanceValue > 1 && balance_factor(node->left) < 0) {
        node->left = rotate_left(node->left);
        return rotate_right(node);
    }
    if (balanceValue < -1 && balance_factor(node->right) <= 0) {
        return rotate_left(node);
    }
    if (balanceValue < -1 && balance_factor(node->right) > 0) {
        node->right = rotate_right(node->right);
        return rotate_left(node);
    }
    return node;
}

DateTreeNode* create_node(const Date& key, std::size_t index) {
    DateTreeNode* node = new DateTreeNode;
    node->key = key;
    node->height = 1;
    node->left = nullptr;
    node->right = nullptr;
    node->indices.clear();
    node->indices.push_back(index);
    return node;
}

DateTreeNode* insert_node(DateTreeNode* node, const Date& key, std::size_t index) {
    if (!node) {
        return create_node(key, index);
    }
    int cmp = compareDate(key, node->key);
    if (cmp < 0) {
        node->left = insert_node(node->left, key, index);
    }
    else if (cmp > 0) {
        node->right = insert_node(node->right, key, index);
    }
    else {
        node->indices.push_back(index);
        return node;
    }
    return balance(node);
}

DateTreeNode* min_node(DateTreeNode* node) {
    while (node && node->left) {
        node = node->left;
    }
    return node;
}

DateTreeNode* remove_node(DateTreeNode* node, const Date& key, bool& removed) {
    if (!node) {
        return nullptr;
    }
    int cmp = compareDate(key, node->key);
    if (cmp < 0) {
        node->left = remove_node(node->left, key, removed);
    }
    else if (cmp > 0) {
        node->right = remove_node(node->right, key, removed);
    }
    else {
        removed = true;
        if (!node->left) {
            DateTreeNode* temp = node->right;
            delete node;
            return temp;
        }
        if (!node->right) {
            DateTreeNode* temp = node->left;
            delete node;
            return temp;
        }
        bool dummy = false;
        DateTreeNode* temp = min_node(node->right);
        node->key = temp->key;
        node->indices = temp->indices;
        node->right = remove_node(node->right, temp->key, dummy);
    }
    return balance(node);
}

DateTreeNode* search_node(DateTreeNode* node, const Date& key) {
    if (!node) {
        return nullptr;
    }
    int cmp = compareDate(key, node->key);
    if (cmp < 0) {
        return search_node(node->left, key);
    }
    if (cmp > 0) {
        return search_node(node->right, key);
    }
    return node;
}

void free_node(DateTreeNode* node) {
    if (!node) {
        return;
    }
    free_node(node->left);
    free_node(node->right);
    delete node;
}

void collect_range(DateTreeNode* node,
                   const Date*   from,
                   const Date*   to,
                   DynamicArray<std::size_t>& out) {
    if (!node) {
        return;
    }
    bool goLeft = true;
    bool goRight = true;
    if (from && compareDate(node->key, *from) < 0) {
        goLeft = false;
    }
    if (to && compareDate(node->key, *to) > 0) {
        goRight = false;
    }
    if (goLeft) {
        collect_range(node->left, from, to, out);
    }
    bool inRange = true;
    if (from && compareDate(node->key, *from) < 0) {
        inRange = false;
    }
    if (to && compareDate(node->key, *to) > 0) {
        inRange = false;
    }
    if (inRange) {
        for (std::size_t i = 0; i < node->indices.size(); ++i) {
            out.push_back(node->indices[i]);
        }
    }
    if (goRight) {
        collect_range(node->right, from, to, out);
    }
}

} // namespace

void date_tree_init(DateTree* tree) {
    tree->root = nullptr;
}

void date_tree_free(DateTree* tree) {
    free_node(tree->root);
    tree->root = nullptr;
}

void date_tree_insert(DateTree* tree, const Date& key, std::size_t index) {
    tree->root = insert_node(tree->root, key, index);
}

bool date_tree_remove_index(DateTree* tree, const Date& key, std::size_t index) {
    DateTreeNode* node = search_node(tree->root, key);
    if (!node) {
        return false;
    }
    for (std::size_t i = 0; i < node->indices.size(); ++i) {
        if (node->indices[i] == index) {
            node->indices.remove_at(i);
            if (node->indices.empty()) {
                bool removed = false;
                tree->root = remove_node(tree->root, key, removed);
                return removed;
            }
            return true;
        }
    }
    return false;
}

bool date_tree_replace_index(DateTree* tree, const Date& key, std::size_t oldIndex, std::size_t newIndex) {
    DateTreeNode* node = search_node(tree->root, key);
    if (!node) {
        return false;
    }
    for (std::size_t i = 0; i < node->indices.size(); ++i) {
        if (node->indices[i] == oldIndex) {
            node->indices[i] = newIndex;
            return true;
        }
    }
    return false;
}

void date_tree_collect_range(const DateTree* tree,
                             const Date*     from,
                             const Date*     to,
                             DynamicArray<std::size_t>& outIndices) {
    collect_range(tree->root, from, to, outIndices);
}
