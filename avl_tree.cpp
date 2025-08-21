#include "avl_tree.h"

#include <iostream>

// ---- Static helpers ------------------------------------------------------

int AVLTree::height(Node* n) {
    return n ? n->height : 0;
}

int AVLTree::maxint(int a, int b) {
    return (a > b) ? a : b;
}

void AVLTree::update_height(Node* n) {
    if (n) {
        n->height = 1 + maxint(height(n->left), height(n->right));
    }
}

int AVLTree::get_balance(Node* n) {
    return n ? height(n->left) - height(n->right) : 0;
}

AVLTree::Node* AVLTree::rotate_right(Node* y) {
    Node* x = y->left;
    Node* T2 = x->right;

    y->left = T2;
    x->right = y;

    update_height(y);
    update_height(x);

    return x;
}

AVLTree::Node* AVLTree::rotate_left(Node* x) {
    Node* y = x->right;
    Node* T2 = y->left;

    x->right = T2;
    y->left = x;

    update_height(x);
    update_height(y);

    return y;
}

int AVLTree::key_compare(const PersonKey& a, const PersonKey& b) {
    if (a.fullName < b.fullName) return -1;
    if (a.fullName > b.fullName) return 1;
    if (a.phoneNumber < b.phoneNumber) return -1;
    if (a.phoneNumber > b.phoneNumber) return 1;
    return 0;
}

AVLTree::Node* AVLTree::create_node(const PersonKey& key, int lineNumber) {
    Node* node = new Node;
    node->key = key;
    node->height = 1;
    node->left = nullptr;
    node->right = nullptr;
    node->lineNumbers.clear();
    node->lineNumbers.push_back(lineNumber);
    return node;
}

AVLTree::Node* AVLTree::balance_node(Node* node) {
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

AVLTree::Node* AVLTree::min_node(Node* node) {
    while (node && node->left) {
        node = node->left;
    }
    return node;
}

AVLTree::Node* AVLTree::insert_node(Node* node, const PersonKey& key, int lineNumber) {
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

AVLTree::Node* AVLTree::remove_node(Node* node, const PersonKey& key, bool& removed) {
    if (!node) return nullptr;

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
            Node* temp = node->right;
            delete node;
            return temp;
        }
        else if (!node->right) {
            Node* temp = node->left;
            delete node;
            return temp;
        }
        bool dummy = false;
        Node* temp = min_node(node->right);
        node->key = temp->key;
        node->lineNumbers = temp->lineNumbers;
        node->right = remove_node(node->right, temp->key, dummy);
    }

    return balance_node(node);
}

AVLTree::Node* AVLTree::search_node(Node* node, const PersonKey& key) {
    if (!node) return nullptr;
    int cmp = key_compare(key, node->key);
    if (cmp < 0) return search_node(node->left, key);
    else if (cmp > 0) return search_node(node->right, key);
    else return node;
}

void AVLTree::free_node(Node* node) {
    if (!node) return;
    free_node(node->left);
    free_node(node->right);
    delete node;
}

void AVLTree::inorder_traversal_nodes(Node* node, std::vector<Node*>& result) {
    if (!node) return;
    inorder_traversal_nodes(node->left, result);
    result.push_back(node);
    inorder_traversal_nodes(node->right, result);
}

void AVLTree::reverse_inorder_traversal_nodes(Node* node, std::vector<Node*>& result) {
    if (!node) return;
    reverse_inorder_traversal_nodes(node->right, result);
    result.push_back(node);
    reverse_inorder_traversal_nodes(node->left, result);
}

static const char* sdown = "  |";
static const char* slast = "  `";
static const char* snone = "   ";

void AVLTree::print_tree_recursive(Node* node, std::vector<const char*>& stems, char childType) {
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

    Node* left = node->left;
    Node* right = node->right;

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

// ---- Public API ----------------------------------------------------------

AVLTree::AVLTree() : root(nullptr) {}

AVLTree::~AVLTree() {
    free_node(root);
    root = nullptr;
}

void AVLTree::insert(const PersonKey& key, int lineNumber) {
    root = insert_node(root, key, lineNumber);
}

bool AVLTree::remove(const PersonKey& key) {
    bool removed = false;
    root = remove_node(root, key, removed);
    return removed;
}

AVLTree::Node* AVLTree::search(const PersonKey& key) const {
    return search_node(root, key);
}

std::vector<AVLTree::Node*> AVLTree::inorderNodes() const {
    std::vector<Node*> result;
    inorder_traversal_nodes(root, result);
    return result;
}

std::vector<AVLTree::Node*> AVLTree::reverseInorderNodes() const {
    std::vector<Node*> result;
    reverse_inorder_traversal_nodes(root, result);
    return result;
}

bool AVLTree::removeLine(const PersonKey& key, int lineNumber) {
    Node* node = search(key);
    if (!node) {
        // Node with such key not found
        return false;
    }

    int index = -1;
    int size = static_cast<int>(node->lineNumbers.size());
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
        return remove(key);
    }

    return true;
}

void AVLTree::printTree() const {
    if (!root) {
        std::cout << "(empty tree)\n";
        return;
    }
    std::vector<const char*> stems;
    print_tree_recursive(root, stems, '\0'); // root has no parent
}

