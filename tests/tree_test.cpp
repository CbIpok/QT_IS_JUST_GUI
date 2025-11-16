#include <gtest/gtest.h>
#include "avl_tree.h"

TEST(AVLTreeTest, InsertAndSearch) {
    AVLTree tree;
    avl_init(&tree);
    std::string license = "TK-25-111111-2023";
    avl_insert(&tree, license, 10);
    AVLNode* node = avl_search(&tree, license);
    ASSERT_NE(node, nullptr);
    ASSERT_EQ(node->listIndices.size(), 1u);
    EXPECT_EQ(node->listIndices.front(), 10u);
    avl_free(&tree);
}

TEST(AVLTreeTest, Remove) {
    AVLTree tree;
    avl_init(&tree);
    std::string licenseOne = "TK-25-111111-2023";
    std::string licenseTwo = "TK-25-222222-2024";
    avl_insert(&tree, licenseOne, 1);
    avl_insert(&tree, licenseTwo, 2);
    EXPECT_TRUE(avl_remove(&tree, licenseOne));
    EXPECT_EQ(avl_search(&tree, licenseOne), nullptr);
    EXPECT_NE(avl_search(&tree, licenseTwo), nullptr);
    avl_free(&tree);
}

TEST(AVLTreeTest, RemoveIndex) {
    AVLTree tree;
    avl_init(&tree);
    std::string license = "TK-25-111111-2023";
    avl_insert(&tree, license, 1);
    avl_insert(&tree, license, 2);
    EXPECT_TRUE(avl_remove_index(&tree, license, 1));
    AVLNode* node = avl_search(&tree, license);
    ASSERT_NE(node, nullptr);
    ASSERT_EQ(node->listIndices.size(), 1u);
    EXPECT_EQ(node->listIndices.front(), 2u);
    EXPECT_TRUE(avl_remove_index(&tree, license, 2));
    EXPECT_EQ(avl_search(&tree, license), nullptr);
    avl_free(&tree);
}

TEST(AVLTreeTest, InorderTraversal) {
    AVLTree tree;
    avl_init(&tree);
    avl_insert(&tree, "b", 0);
    avl_insert(&tree, "a", 1);
    avl_insert(&tree, "c", 2);
    auto nodes = avl_inorder_nodes(&tree);
    ASSERT_EQ(nodes.size(), 3u);
    EXPECT_EQ(nodes[0]->license, "a");
    EXPECT_EQ(nodes[1]->license, "b");
    EXPECT_EQ(nodes[2]->license, "c");
    avl_free(&tree);
}

TEST(AVLTreeTest, DumpToStringContainsBranching) {
    AVLTree tree;
    avl_init(&tree);
    avl_insert(&tree, "m", 0);
    avl_insert(&tree, "a", 1);
    avl_insert(&tree, "z", 2);

    std::string dump = avl_tree_to_string(&tree);
    EXPECT_NE(dump.find("m"), std::string::npos);
    EXPECT_NE(dump.find("|--"), std::string::npos);
    EXPECT_NE(dump.find("`--"), std::string::npos);
    EXPECT_NE(dump.find(" (L)"), std::string::npos);
    EXPECT_NE(dump.find(" (R)"), std::string::npos);

    avl_free(&tree);
}
