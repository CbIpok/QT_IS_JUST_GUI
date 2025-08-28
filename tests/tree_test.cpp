#include <gtest/gtest.h>
#include "avl_tree.h"

TEST(AVLTreeTest, InsertAndSearch) {
    AVLTree tree; avl_init(&tree);
    OrderKey k{"TK-25-111111-2023", "Ul. Lesnaya"};
    avl_insert(&tree, k, 10);
    AVLNode* node = avl_search(&tree, k);
    ASSERT_NE(node, nullptr);
    ASSERT_EQ(node->lineNumbers.size(), 1u);
    EXPECT_EQ(node->lineNumbers[0], 10);
    avl_free(&tree);
}

TEST(AVLTreeTest, Remove) {
    AVLTree tree; avl_init(&tree);
    OrderKey k1{"TK-25-111111-2023", "Ul. Lesnaya"};
    OrderKey k2{"TK-25-222222-2024", "Ul. Lenina"};
    avl_insert(&tree, k1, 1);
    avl_insert(&tree, k2, 2);
    EXPECT_TRUE(avl_remove(&tree, k1));
    EXPECT_EQ(avl_search(&tree, k1), nullptr);
    EXPECT_NE(avl_search(&tree, k2), nullptr);
    avl_free(&tree);
}

TEST(AVLTreeTest, RemoveLine) {
    AVLTree tree; avl_init(&tree);
    OrderKey k{"TK-25-111111-2023", "Ul. Lesnaya"};
    avl_insert(&tree, k, 1);
    avl_insert(&tree, k, 2);
    EXPECT_TRUE(avl_remove_line(&tree, k, 1));
    AVLNode* node = avl_search(&tree, k);
    ASSERT_NE(node, nullptr);
    ASSERT_EQ(node->lineNumbers.size(), 1u);
    EXPECT_EQ(node->lineNumbers[0], 2);
    EXPECT_TRUE(avl_remove_line(&tree, k, 2));
    EXPECT_EQ(avl_search(&tree, k), nullptr);
    avl_free(&tree);
}

TEST(AVLTreeTest, InorderTraversal) {
    AVLTree tree; avl_init(&tree);
    avl_insert(&tree, {"b", "2"}, 0);
    avl_insert(&tree, {"a", "1"}, 0);
    avl_insert(&tree, {"c", "3"}, 0);
    auto nodes = avl_inorder_nodes(&tree);
    ASSERT_EQ(nodes.size(), 3u);
    EXPECT_EQ(nodes[0]->key.licenseNumber, "a");
    EXPECT_EQ(nodes[1]->key.licenseNumber, "b");
    EXPECT_EQ(nodes[2]->key.licenseNumber, "c");
    avl_free(&tree);
}
