#ifndef AVL_TREE_H
#define AVL_TREE_H

#include <cstddef>
#include <string>
#include <vector>

#include "order_record.hpp"

struct AVLNode {
    OrderRecord key;
    int         height;
    AVLNode*    left;
    AVLNode*    right;
    std::vector<std::size_t> listIndices;
};

struct AVLTree {
    AVLNode* root;
};

void avl_init(AVLTree* tree);

void avl_insert(AVLTree* tree, const OrderRecord& key, std::size_t listIndex);

bool avl_remove(AVLTree* tree, const OrderRecord& key);

AVLNode* avl_search(AVLTree* tree, const OrderRecord& key);
const AVLNode* avl_search(const AVLTree* tree, const OrderRecord& key);

std::vector<AVLNode*> avl_inorder_nodes(const AVLTree* tree);
std::vector<AVLNode*> avl_reverse_inorder_nodes(const AVLTree* tree);

void avl_free(AVLTree* tree);

bool avl_remove_index(AVLTree* tree, const OrderRecord& key, std::size_t listIndex);

bool avl_replace_index(AVLTree* tree, const OrderRecord& key, std::size_t oldIndex, std::size_t newIndex);

void avl_print_tree(const AVLTree* tree);
std::string avl_tree_to_string(const AVLTree* tree);

#endif
