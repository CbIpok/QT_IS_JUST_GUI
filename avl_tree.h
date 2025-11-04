#ifndef AVL_TREE_H
#define AVL_TREE_H

#include <cstddef>
#include <string>

#include "dynamic_array.hpp"

struct AVLNode {
    std::string             license;
    int                     height;
    AVLNode*                left;
    AVLNode*                right;
    DynamicArray<std::size_t> listIndices;
};

struct AVLTree {
    AVLNode* root;
};

void avl_init(AVLTree* tree);

void avl_insert(AVLTree* tree, const std::string& license, std::size_t listIndex);

bool avl_remove(AVLTree* tree, const std::string& license);

AVLNode* avl_search(AVLTree* tree, const std::string& license);
const AVLNode* avl_search(const AVLTree* tree, const std::string& license);

DynamicArray<AVLNode*> avl_inorder_nodes(const AVLTree* tree);
void avl_free(AVLTree* tree);

bool avl_remove_index(AVLTree* tree, const std::string& license, std::size_t listIndex);

bool avl_replace_index(AVLTree* tree, const std::string& license, std::size_t oldIndex, std::size_t newIndex);

std::string avl_tree_to_string(const AVLTree* tree);

#endif
