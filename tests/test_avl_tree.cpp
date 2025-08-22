#include <cassert>
#include <iostream>
#include "../avl_tree.h"

int main() {
    AVLTree tree;
    tree.insert({"Ivanov", 100}, 1);
    tree.insert({"Petrov", 200}, 2);
    tree.insert({"Sidorov", 150}, 3);
    tree.insert({"Petrov", 200}, 5); // duplicate key adds line

    // search existing key
    auto node = tree.search({"Petrov", 200});
    assert(node && node->lineNumbers.size() == 2);

    // check in-order traversal (ascending)
    auto inorder = tree.inorderNodes();
    assert(inorder.size() == 3);
    assert(inorder[0]->key.fullName == "Ivanov");
    assert(inorder[1]->key.fullName == "Petrov");
    assert(inorder[2]->key.fullName == "Sidorov");

    // check reverse in-order (descending)
    auto rev = tree.reverseInorderNodes();
    assert(rev[0]->key.fullName == "Sidorov");
    assert(rev[2]->key.fullName == "Ivanov");

    // remove specific line for a key
    assert(tree.removeLine({"Petrov", 200}, 5));
    node = tree.search({"Petrov", 200});
    assert(node && node->lineNumbers.size() == 1 && node->lineNumbers[0] == 2);

    // remove entire node
    assert(tree.remove({"Petrov", 200}));
    assert(tree.search({"Petrov", 200}) == nullptr);

    // removing last line deletes node
    assert(tree.removeLine({"Ivanov", 100}, 1));
    assert(tree.search({"Ivanov", 100}) == nullptr);

    auto remaining = tree.inorderNodes();
    assert(remaining.size() == 1);
    assert(remaining[0]->key.fullName == "Sidorov");

    std::cout << "AVL tree tests passed\n";
    return 0;
}
