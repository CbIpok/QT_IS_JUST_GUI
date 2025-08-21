#include "avl_tree.h"
#include <iostream>

AVLTree::Node::Node(const PersonKey& k, int line)
    : key(k), height(1), left(nullptr), right(nullptr) {
    lineNumbers.push_back(line);
}

AVLTree::AVLTree() : root(nullptr) {}

AVLTree::~AVLTree() { freeNode(root); }

int AVLTree::height(Node* n) { return n ? n->height : 0; }

int AVLTree::max(int a, int b) { return a > b ? a : b; }

void AVLTree::updateHeight(Node* n) {
    if (n) n->height = 1 + max(height(n->left), height(n->right));
}

int AVLTree::getBalance(Node* n) { return n ? height(n->left) - height(n->right) : 0; }

AVLTree::Node* AVLTree::rotateRight(Node* y) {
    Node* x = y->left;
    Node* T2 = x->right;
    y->left = T2;
    x->right = y;
    updateHeight(y);
    updateHeight(x);
    return x;
}

AVLTree::Node* AVLTree::rotateLeft(Node* x) {
    Node* y = x->right;
    Node* T2 = y->left;
    x->right = T2;
    y->left = x;
    updateHeight(x);
    updateHeight(y);
    return y;
}

int AVLTree::keyCompare(const PersonKey& a, const PersonKey& b) const {
    if (a.fullName < b.fullName) return -1;
    if (a.fullName > b.fullName) return 1;
    if (a.phoneNumber < b.phoneNumber) return -1;
    if (a.phoneNumber > b.phoneNumber) return 1;
    return 0;
}

AVLTree::Node* AVLTree::balanceNode(Node* node) {
    updateHeight(node);
    int bal = getBalance(node);
    if (bal > 1 && getBalance(node->left) >= 0) return rotateRight(node);
    if (bal > 1 && getBalance(node->left) < 0) {
        node->left = rotateLeft(node->left);
        return rotateRight(node);
    }
    if (bal < -1 && getBalance(node->right) <= 0) return rotateLeft(node);
    if (bal < -1 && getBalance(node->right) > 0) {
        node->right = rotateRight(node->right);
        return rotateLeft(node);
    }
    return node;
}

AVLTree::Node* AVLTree::minNode(Node* node) {
    while (node && node->left)
        node = node->left;
    return node;
}

AVLTree::Node* AVLTree::insertNode(Node* node, const PersonKey& key, int lineNumber) {
    if (!node) return new Node(key, lineNumber);
    int cmp = keyCompare(key, node->key);
    if (cmp < 0)
        node->left = insertNode(node->left, key, lineNumber);
    else if (cmp > 0)
        node->right = insertNode(node->right, key, lineNumber);
    else {
        node->lineNumbers.push_back(lineNumber);
        return node;
    }
    return balanceNode(node);
}

void AVLTree::insert(const PersonKey& key, int lineNumber) {
    root = insertNode(root, key, lineNumber);
}

AVLTree::Node* AVLTree::removeNode(Node* node, const PersonKey& key, bool& removed) {
    if (!node) return nullptr;
    int cmp = keyCompare(key, node->key);
    if (cmp < 0) {
        node->left = removeNode(node->left, key, removed);
    } else if (cmp > 0) {
        node->right = removeNode(node->right, key, removed);
    } else {
        removed = true;
        if (!node->left || !node->right) {
            Node* temp = node->left ? node->left : node->right;
            if (!temp) {
                temp = node;
                node = nullptr;
            } else {
                *node = *temp;
            }
            delete temp;
        } else {
            Node* temp = minNode(node->right);
            node->key = temp->key;
            node->lineNumbers = temp->lineNumbers;
            node->right = removeNode(node->right, temp->key, removed);
        }
    }
    if (!node) return node;
    return balanceNode(node);
}

bool AVLTree::remove(const PersonKey& key) {
    bool removed = false;
    root = removeNode(root, key, removed);
    return removed;
}

AVLTree::Node* AVLTree::searchNode(Node* node, const PersonKey& key) const {
    if (!node) return nullptr;
    int cmp = keyCompare(key, node->key);
    if (cmp < 0) return searchNode(node->left, key);
    if (cmp > 0) return searchNode(node->right, key);
    return node;
}

AVLTree::Node* AVLTree::search(const PersonKey& key) const {
    return searchNode(root, key);
}

void AVLTree::inorderTraversal(Node* node, std::vector<Node*>& result) const {
    if (!node) return;
    inorderTraversal(node->left, result);
    result.push_back(node);
    inorderTraversal(node->right, result);
}

std::vector<AVLTree::Node*> AVLTree::inorderNodes() const {
    std::vector<Node*> res;
    inorderTraversal(root, res);
    return res;
}

void AVLTree::reverseInorderTraversal(Node* node, std::vector<Node*>& result) const {
    if (!node) return;
    reverseInorderTraversal(node->right, result);
    result.push_back(node);
    reverseInorderTraversal(node->left, result);
}

std::vector<AVLTree::Node*> AVLTree::reverseInorderNodes() const {
    std::vector<Node*> res;
    reverseInorderTraversal(root, res);
    return res;
}

void AVLTree::freeNode(Node* node) {
    if (!node) return;
    freeNode(node->left);
    freeNode(node->right);
    delete node;
}

bool AVLTree::removeLine(const PersonKey& key, int lineNumber) {
    Node* node = search(key);
    if (!node) return false;
    int index = -1;
    for (size_t i = 0; i < node->lineNumbers.size(); ++i) {
        if (node->lineNumbers[i] == lineNumber) {
            index = static_cast<int>(i);
            break;
        }
    }
    if (index == -1) return false;
    node->lineNumbers.erase(node->lineNumbers.begin() + index);
    if (node->lineNumbers.empty())
        return remove(key);
    return true;
}

void AVLTree::printTreeRecursive(Node* node, std::vector<const char*>& stems, char childType) const {
    if (!node) return;
    for (const char* s : stems)
        std::cout << s;
    std::cout << "--";
    if (childType == 'L') std::cout << "(L) ";
    else if (childType == 'R') std::cout << "(R) ";
    std::cout << node->key.fullName << " " << node->key.phoneNumber;
    if (!node->lineNumbers.empty()) {
        std::cout << " [";
        for (size_t i = 0; i < node->lineNumbers.size(); ++i) {
            std::cout << node->lineNumbers[i];
            if (i + 1 < node->lineNumbers.size()) std::cout << ",";
        }
        std::cout << "]";
    }
    std::cout << "\n";
    Node* left = node->left;
    Node* right = node->right;
    if (!left && !right) return;
    size_t oldSize = stems.size();
    static const char* sdown = "  |";
    static const char* slast = "  `";
    if (left && right) {
        stems.push_back(sdown);
        printTreeRecursive(left, stems, 'L');
        stems.pop_back();
        stems.push_back(slast);
        printTreeRecursive(right, stems, 'R');
        stems.pop_back();
    } else if (left) {
        stems.push_back(slast);
        printTreeRecursive(left, stems, 'L');
        stems.pop_back();
    } else {
        stems.push_back(slast);
        printTreeRecursive(right, stems, 'R');
        stems.pop_back();
    }
    stems.resize(oldSize);
}

void AVLTree::printTree() const {
    if (!root) {
        std::cout << "(empty tree)\n";
        return;
    }
    std::vector<const char*> stems;
    printTreeRecursive(root, stems, '\0');
}

