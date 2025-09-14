#include <gtest/gtest.h>
#include "../avl_tree.h"
#include "../doubly_linked_array.hpp"

TEST(AVLTree, FullWorkflow) {
    DoublyLinkedArray<PersonKey> storage;
    AVLTree tree(storage);
    tree.insert({"Ivanov", 100}, 1);
    tree.insert({"Petrov", 200}, 2);
    tree.insert({"Sidorov", 150}, 3);
    tree.insert({"Petrov", 200}, 5); // duplicate key adds line

    // search existing key
    auto node = tree.search({"Petrov", 200});
    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->lineNumbers.size(), 2);

    // check in-order traversal (ascending)
    auto inorder = tree.inorderNodes();
    ASSERT_EQ(inorder.size(), 3);
    EXPECT_EQ(tree.getKey(inorder[0]).fullName, "Ivanov");
    EXPECT_EQ(tree.getKey(inorder[1]).fullName, "Petrov");
    EXPECT_EQ(tree.getKey(inorder[2]).fullName, "Sidorov");

    // check reverse in-order (descending)
    auto rev = tree.reverseInorderNodes();
    EXPECT_EQ(tree.getKey(rev[0]).fullName, "Sidorov");
    EXPECT_EQ(tree.getKey(rev[2]).fullName, "Ivanov");

    // remove specific line for a key
    EXPECT_TRUE(tree.removeLine({"Petrov", 200}, 5));
    node = tree.search({"Petrov", 200});
    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->lineNumbers.size(), 1);
    EXPECT_EQ(node->lineNumbers[0], 2);

    // remove entire node
    EXPECT_TRUE(tree.remove({"Petrov", 200}));
    EXPECT_EQ(tree.search({"Petrov", 200}), nullptr);

    // removing last line deletes node
    EXPECT_TRUE(tree.removeLine({"Ivanov", 100}, 1));
    EXPECT_EQ(tree.search({"Ivanov", 100}), nullptr);

    auto remaining = tree.inorderNodes();
    ASSERT_EQ(remaining.size(), 1);
    EXPECT_EQ(tree.getKey(remaining[0]).fullName, "Sidorov");
}


