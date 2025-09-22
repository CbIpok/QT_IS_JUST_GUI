#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

#include "avl_tree.h"

static int height(AVLNode* n) {
    return n ? n->height : 0;
}

static int maxint(int a, int b) {
    return (a > b) ? a : b;
}

static void update_height(AVLNode* n) {
    if (n) {
        n->height = 1 + maxint(height(n->left), height(n->right));
    }
}

static int get_balance(AVLNode* n) {
    return n ? height(n->left) - height(n->right) : 0;
}

static AVLNode* rotate_right(AVLNode* y) {
    AVLNode* x = y->left;
    AVLNode* T2 = x->right;

    y->left = T2;
    x->right = y;

    update_height(y);
    update_height(x);

    return x;
}

static AVLNode* rotate_left(AVLNode* x) {
    AVLNode* y = x->right;
    AVLNode* T2 = y->left;

    x->right = T2;
    y->left = x;

    update_height(x);
    update_height(y);

    return y;
}

static int key_compare(const OrderRecord& a, const OrderRecord& b) {
    if (a.licenseNumber < b.licenseNumber) return -1;
    if (a.licenseNumber > b.licenseNumber) return 1;
    if (a.address < b.address) return -1;
    if (a.address > b.address) return 1;
    return 0;
}

static AVLNode* create_node(const OrderRecord& key, std::size_t listIndex) {
    AVLNode* node = new AVLNode;
    node->key = key;
    node->height = 1;
    node->left = 0;
    node->right = 0;
    node->listIndices.clear();
    node->listIndices.push_back(listIndex);
    return node;
}

static AVLNode* balance_node(AVLNode* node) {
    update_height(node);
    int bal = get_balance(node);

    if (bal > 1 && get_balance(node->left) >= 0)
        return rotate_right(node);

    if (bal > 1 && get_balance(node->left) < 0) {
        node->left = rotate_left(node->left);
        return rotate_right(node);
    }

    if (bal < -1 && get_balance(node->right) <= 0)
        return rotate_left(node);

    if (bal < -1 && get_balance(node->right) > 0) {
        node->right = rotate_right(node->right);
        return rotate_left(node);
    }

    return node;
}

static AVLNode* min_node(AVLNode* node) {
    while (node && node->left) {
        node = node->left;
    }
    return node;
}

static AVLNode* insert_node(AVLNode* node, const OrderRecord& key, std::size_t listIndex) {
    if (!node) return create_node(key, listIndex);

    int cmp = key_compare(key, node->key);
    if (cmp < 0) {
        node->left = insert_node(node->left, key, listIndex);
    }
    else if (cmp > 0) {
        node->right = insert_node(node->right, key, listIndex);
    }
    else {
        node->listIndices.push_back(listIndex);
        return node;
    }

    return balance_node(node);
}

static AVLNode* remove_node(AVLNode* node, const OrderRecord& key, bool& removed) {
    if (!node) return 0;

    int cmp = key_compare(key, node->key);
    if (cmp < 0) {
        node->left = remove_node(node->left, key, removed);
    }
    else if (cmp > 0) {
        node->right = remove_node(node->right, key, removed);
    }
    else {
        removed = true;
        if (!node->left) {
            AVLNode* temp = node->right;
            delete node;
            return temp;
        }
        else if (!node->right) {
            AVLNode* temp = node->left;
            delete node;
            return temp;
        }
        bool dummy = false;
        AVLNode* temp = min_node(node->right);
        node->key = temp->key;
        node->listIndices = temp->listIndices;
        node->right = remove_node(node->right, temp->key, dummy);
    }

    return balance_node(node);
}

static AVLNode* search_node(AVLNode* node, const OrderRecord& key) {
    if (!node) return 0;
    int cmp = key_compare(key, node->key);
    if (cmp < 0) return search_node(node->left, key);
    else if (cmp > 0) return search_node(node->right, key);
    else return node;
}

static void free_node(AVLNode* node) {
    if (!node) return;
    free_node(node->left);
    free_node(node->right);
    delete node;
}

static void inorder_traversal_nodes(AVLNode* node, std::vector<AVLNode*>& result) {
    if (!node) return;
    inorder_traversal_nodes(node->left, result);
    result.push_back(node);
    inorder_traversal_nodes(node->right, result);
}

void avl_init(AVLTree* tree) {
    tree->root = 0;
}

void avl_insert(AVLTree* tree, const OrderRecord& key, std::size_t listIndex) {
    tree->root = insert_node(tree->root, key, listIndex);
}

bool avl_remove(AVLTree* tree, const OrderRecord& key) {
    bool removed = false;
    tree->root = remove_node(tree->root, key, removed);
    return removed;
}

AVLNode* avl_search(AVLTree* tree, const OrderRecord& key) {
    return search_node(tree->root, key);
}

const AVLNode* avl_search(const AVLTree* tree, const OrderRecord& key) {
    return search_node(tree->root, key);
}

std::vector<AVLNode*> avl_inorder_nodes(const AVLTree* tree) {
    std::vector<AVLNode*> result;
    inorder_traversal_nodes(tree->root, result);
    return result;
}

void avl_free(AVLTree* tree) {
    free_node(tree->root);
    tree->root = 0;
}

bool avl_remove_index(AVLTree* tree, const OrderRecord& key, std::size_t listIndex) {
    AVLNode* node = avl_search(tree, key);
    if (!node) {
        return false;
    }

    auto it = std::find(node->listIndices.begin(), node->listIndices.end(), listIndex);
    if (it == node->listIndices.end()) {
        return false;
    }

    node->listIndices.erase(it);

    if (node->listIndices.empty()) {
        return avl_remove(tree, key);
    }

    return true;
}

bool avl_replace_index(AVLTree* tree, const OrderRecord& key, std::size_t oldIndex, std::size_t newIndex) {
    AVLNode* node = avl_search(tree, key);
    if (!node) {
        return false;
    }

    for (std::size_t& idx : node->listIndices) {
        if (idx == oldIndex) {
            idx = newIndex;
            return true;
        }
    }

    return false;
}

namespace {

void tree_to_stream(const AVLNode* node,
                    std::ostream&     out,
                    const std::string& prefix,
                    bool                isTail,
                    bool                isRoot) {
    if (!node) {
        return;
    }

    out << prefix;
    if (!isRoot) {
        out << (isTail ? "`--" : "|--");
    }

    out << node->key.licenseNumber << " | "
        << node->key.address << " | "
        << node->key.cost << " | "
        << node->key.date;

    if (!node->listIndices.empty()) {
        out << " [";
        for (std::size_t i = 0; i < node->listIndices.size(); ++i) {
            out << node->listIndices[i];
            if (i + 1 < node->listIndices.size()) {
                out << ",";
            }
        }
        out << "]";
    }
    out << '\n';

    std::vector<const AVLNode*> children;
    if (node->left) {
        children.push_back(node->left);
    }
    if (node->right) {
        children.push_back(node->right);
    }

    if (children.empty()) {
        return;
    }

    std::string childPrefix;
    if (!isRoot) {
        childPrefix = prefix + (isTail ? "    " : "|   ");
    }

    for (std::size_t i = 0; i < children.size(); ++i) {
        bool childIsTail = (i + 1 == children.size());
        if (isRoot) {
            tree_to_stream(children[i], out, "", childIsTail, false);
        }
        else {
            tree_to_stream(children[i], out, childPrefix, childIsTail, false);
        }
    }
}

} // namespace

std::string avl_tree_to_string(const AVLTree* tree) {
    std::ostringstream out;
    if (!tree || !tree->root) {
        out << "(empty tree)\n";
        return out.str();
    }

    tree_to_stream(tree->root, out, "", true, true);
    return out.str();
}

