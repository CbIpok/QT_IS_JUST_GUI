#include <string>
#include <vector>
#include "avl_tree.h"
#include <iostream>

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

static int key_compare(const PersonKey& a, const PersonKey& b) {
    if (a.fullName < b.fullName) return -1;
    if (a.fullName > b.fullName) return 1;
    if (a.phoneNumber < b.phoneNumber) return -1;
    if (a.phoneNumber > b.phoneNumber) return 1;
    return 0;
}

static AVLNode* create_node(const PersonKey& key, int lineNumber) {
    AVLNode* node = new AVLNode;
    node->key = key;
    node->height = 1;
    node->left = 0;
    node->right = 0;
    node->lineNumbers.clear();
    node->lineNumbers.push_back(lineNumber);
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

static AVLNode* insert_node(AVLNode* node, const PersonKey& key, int lineNumber) {
    if (!node) return create_node(key, lineNumber);

    int cmp = key_compare(key, node->key);
    if (cmp < 0) {
        node->left = insert_node(node->left, key, lineNumber);
    }
    else if (cmp > 0) {
        node->right = insert_node(node->right, key, lineNumber);
    }
    else {
        // Key already exists, append the line number
        node->lineNumbers.push_back(lineNumber);
        return node;
    }

    return balance_node(node);
}

static AVLNode* remove_node(AVLNode* node, const PersonKey& key, bool& removed) {
    if (!node) return 0;

    int cmp = key_compare(key, node->key);
    if (cmp < 0) {
        node->left = remove_node(node->left, key, removed);
    }
    else if (cmp > 0) {
        node->right = remove_node(node->right, key, removed);
    }
    else {
        // Node found
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
        node->lineNumbers = temp->lineNumbers;
        node->right = remove_node(node->right, temp->key, dummy);
    }

    return balance_node(node);
}

static AVLNode* search_node(AVLNode* node, const PersonKey& key) {
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

static void reverse_inorder_traversal_nodes(AVLNode* node, std::vector<AVLNode*>& result) {
    if (!node) return;
    reverse_inorder_traversal_nodes(node->right, result);
    result.push_back(node);
    reverse_inorder_traversal_nodes(node->left, result);
}

// Public functions
void avl_init(AVLTree* tree) {
    tree->root = 0;
}

void avl_insert(AVLTree* tree, const PersonKey& key, int lineNumber) {
    tree->root = insert_node(tree->root, key, lineNumber);
}

bool avl_remove(AVLTree* tree, const PersonKey& key) {
    bool removed = false;
    tree->root = remove_node(tree->root, key, removed);
    return removed;
}

AVLNode* avl_search(AVLTree* tree, const PersonKey& key) {
    return search_node(tree->root, key);
}

std::vector<AVLNode*> avl_inorder_nodes(const AVLTree* tree) {
    std::vector<AVLNode*> result;
    inorder_traversal_nodes(tree->root, result);
    return result;
}

std::vector<AVLNode*> avl_reverse_inorder_nodes(const AVLTree* tree) {
    std::vector<AVLNode*> result;
    reverse_inorder_traversal_nodes(tree->root, result);
    return result;
}

void avl_free(AVLTree* tree) {
    free_node(tree->root);
    tree->root = 0;
}

bool avl_remove_line(AVLTree* tree, const PersonKey& key, int lineNumber) {
    AVLNode* node = avl_search(tree, key);
    if (!node) {
        // Node with such key not found
        return false;
    }

    int index = -1;
    int size = node->lineNumbers.size();
    for (int i = 0; i < size; i++) {
        if (node->lineNumbers[i] == lineNumber) {
            index = i;
            break;
        }
    }

    if (index == -1) {
        // This line number not found for this key
        return false;
    }

    // Remove the line number by shifting
    for (int i = index; i < size - 1; i++) {
        node->lineNumbers[i] = node->lineNumbers[i + 1];
    }
    node->lineNumbers.pop_back();

    // If empty after removal, remove the whole node
    if (node->lineNumbers.empty()) {
        return avl_remove(tree, key);
    }

    return true;
}

static const char* sdown = "  |";
static const char* slast = "  `";
static const char* snone = "   ";

static void print_tree_recursive(AVLNode* node, std::vector<const char*>& stems, char childType) {
    if (!node) return;

    for (std::size_t i = 0; i < stems.size(); i++) {
        std::cout << stems[i];
    }

    // Print current node
    std::cout << "--";
    if (childType == 'L') {
        std::cout << "(L) ";
    }
    else if (childType == 'R') {
        std::cout << "(R) ";
    }

    std::cout << node->key.fullName << " " << node->key.phoneNumber;
    if (!node->lineNumbers.empty()) {
        std::cout << " [";
        for (std::size_t i = 0; i < node->lineNumbers.size(); i++) {
            std::cout << node->lineNumbers[i];
            if (i + 1 < node->lineNumbers.size()) std::cout << ",";
        }
        std::cout << "]";
    }
    std::cout << "\n";

    AVLNode* left = node->left;
    AVLNode* right = node->right;

    if (!left && !right) return;

    std::size_t oldSize = stems.size();

    if (left && right) {
        // Left child: not last
        stems.push_back(sdown);
        print_tree_recursive(left, stems, 'L');
        stems.pop_back();

        // Right child: last
        stems.push_back(slast);
        print_tree_recursive(right, stems, 'R');
        stems.pop_back();
    }
    else if (left) {
        // Only left child
        stems.push_back(slast);
        print_tree_recursive(left, stems, 'L');
        stems.pop_back();
    }
    else {
        // Only right child
        stems.push_back(slast);
        print_tree_recursive(right, stems, 'R');
        stems.pop_back();
    }

    stems.resize(oldSize);
}

void avl_print_tree(const AVLTree* tree) {
    if (!tree || !tree->root) {
        std::cout << "(empty tree)\n";
        return;
    }
    std::vector<const char*> stems;
    print_tree_recursive(tree->root, stems, '\0'); // root has no parent
}