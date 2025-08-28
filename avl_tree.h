#ifndef AVL_TREE_H
#define AVL_TREE_H

#include <string>
#include <vector>

struct OrderKey {
    std::string licenseNumber;
    std::string address;
};

struct AVLNode {
    OrderKey key;
    int height;
    AVLNode* left;
    AVLNode* right;
    std::vector<int> lineNumbers;
};

struct AVLTree {
    AVLNode* root;
};

void avl_init(AVLTree* tree);

void avl_insert(AVLTree* tree, const OrderKey& key, int lineNumber);

bool avl_remove(AVLTree* tree, const OrderKey& key);

AVLNode* avl_search(AVLTree* tree, const OrderKey& key);

std::vector<AVLNode*> avl_inorder_nodes(const AVLTree* tree);
std::vector<AVLNode*> avl_reverse_inorder_nodes(const AVLTree* tree);

void avl_free(AVLTree* tree);

bool avl_remove_line(AVLTree* tree, const OrderKey& key, int lineNumber);

void avl_print_tree(const AVLTree* tree);

#endif
