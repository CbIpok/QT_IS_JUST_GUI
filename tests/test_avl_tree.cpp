#include <gtest/gtest.h>
#include "../avl_tree.h"

TEST(AVLTree, FullWorkflow) {
    AVLTree tree;
    tree.insert({"TK-25-111111-2023", "Ул. Лесная, дом 10", 300, "02 jan 2025", 1});
    tree.insert({"TK-25-222222-2024", "Ул. Ленина, дом 25", 500, "07 jan 2025", 2});
    tree.insert({"TK-25-333333-2022", "Ул. Победы, дом 50", 250, "12 jan 2025", 3});

    // search existing order
    auto node = tree.search("TK-25-222222-2024", "07 jan 2025");
    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->data.cost, 500);

    // check in-order traversal (ascending by license+date)
    auto inorder = tree.inorderNodes();
    ASSERT_EQ(inorder.size(), 3);
    EXPECT_EQ(inorder[0]->data.licenseNumber, "TK-25-111111-2023");
    EXPECT_EQ(inorder[1]->data.licenseNumber, "TK-25-222222-2024");
    EXPECT_EQ(inorder[2]->data.licenseNumber, "TK-25-333333-2022");

    // check reverse in-order (descending)
    auto rev = tree.reverseInorderNodes();
    EXPECT_EQ(rev[0]->data.licenseNumber, "TK-25-333333-2022");
    EXPECT_EQ(rev[2]->data.licenseNumber, "TK-25-111111-2023");

    // remove specific order
    EXPECT_TRUE(tree.remove({"TK-25-222222-2024", "", 0, "07 jan 2025", 0}));
    EXPECT_EQ(tree.search("TK-25-222222-2024", "07 jan 2025"), nullptr);
}
