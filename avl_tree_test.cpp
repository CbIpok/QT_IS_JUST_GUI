#include "avl_tree.h"
#include <cassert>

int main() {
    AVLTree tree;
    // Insert nodes
    tree.insert({"Alice", 111}, 1);
    tree.insert({"Bob", 222}, 2);
    tree.insert({"Charlie", 333}, 3);

    // Search existing and non-existing
    assert(tree.search({"Bob", 222}) != nullptr);
    assert(tree.search({"Eve", 444}) == nullptr);

    // Remove node
    assert(tree.remove({"Bob", 222}));
    assert(tree.search({"Bob", 222}) == nullptr);

    // Insert duplicate key with different line numbers
    tree.insert({"Alice", 111}, 4);
    auto node = tree.search({"Alice", 111});
    assert(node && node->lineNumbers.size() == 2);

    // Remove single line
    assert(tree.removeLine({"Alice", 111}, 1));
    node = tree.search({"Alice", 111});
    assert(node && node->lineNumbers.size() == 1 && node->lineNumbers[0] == 4);

    // Remove last line -> node should be removed
    assert(tree.removeLine({"Alice", 111}, 4));
    assert(tree.search({"Alice", 111}) == nullptr);

    // Reverse inorder traversal should produce descending keys
    tree.insert({"D", 1}, 10);
    tree.insert({"B", 2}, 20);
    tree.insert({"C", 3}, 30);
    tree.insert({"A", 4}, 40);
    auto nodesAsc = tree.inorderNodes();
    auto nodesDesc = tree.reverseInorderNodes();
    assert(nodesAsc.front()->key.fullName == "A");
    assert(nodesDesc.front()->key.fullName == "D");

    return 0;
}
