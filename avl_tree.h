#ifndef AVL_TREE_H
#define AVL_TREE_H

#include <vector>
#include "order_record.hpp"

struct AVLNode {
    OrderRecord key;
    int height;
    AVLNode* left;
    AVLNode* right;
    std::vector<int> lineNumbers;
};

struct AVLTree {
    AVLNode* root;
};

void avl_init(AVLTree* tree);

void avl_insert(AVLTree* tree, const OrderRecord& key, int lineNumber);

bool avl_remove(AVLTree* tree, const OrderRecord& key);

AVLNode* avl_search(AVLTree* tree, const OrderRecord& key);

std::vector<AVLNode*> avl_inorder_nodes(const AVLTree* tree);
std::vector<AVLNode*> avl_reverse_inorder_nodes(const AVLTree* tree);

void avl_free(AVLTree* tree);

bool avl_remove_line(AVLTree* tree, const OrderRecord& key, int lineNumber);

void avl_print_tree(const AVLTree* tree);

#endif
