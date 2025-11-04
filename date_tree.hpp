#pragma once

#include "date_utils.hpp"
#include "dynamic_array.hpp"

struct DateTreeNode {
    Date             key;
    int              height;
    DateTreeNode*    left;
    DateTreeNode*    right;
    DynamicArray<std::size_t> indices;
};

struct DateTree {
    DateTreeNode* root;
};

void date_tree_init(DateTree* tree);
void date_tree_free(DateTree* tree);

void date_tree_insert(DateTree* tree, const Date& key, std::size_t index);
bool date_tree_remove_index(DateTree* tree, const Date& key, std::size_t index);
bool date_tree_replace_index(DateTree* tree, const Date& key, std::size_t oldIndex, std::size_t newIndex);

void date_tree_collect_range(const DateTree* tree,
                             const Date*     from,
                             const Date*     to,
                             DynamicArray<std::size_t>& outIndices);

