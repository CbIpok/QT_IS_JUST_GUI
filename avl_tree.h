#ifndef AVL_TREE_H
#define AVL_TREE_H

#include <string>
#include <vector>
#include "record2.hpp"

// Object oriented AVL tree implementation storing taxi orders.
// Orders are ordered by (licenseNumber, date).
class AVLTree {
public:
    struct Node {
        OrderRecord data;    // full record of the order
        int    height;  // height of the subtree
        Node*  left;    // left child
        Node*  right;   // right child

        explicit Node(const OrderRecord& r);
    };

    AVLTree();
    ~AVLTree();

    void insert(const OrderRecord& rec);
    bool remove(const OrderRecord& rec);
    Node* search(const std::string& licenseNumber, const std::string& date) const;

    std::vector<Node*> inorderNodes() const;
    std::vector<Node*> reverseInorderNodes() const;

    void printTree() const;

private:
    Node* root;

    static int  height(Node* n);
    static int  max(int a, int b);
    static void updateHeight(Node* n);
    static int  getBalance(Node* n);
    static Node* rotateRight(Node* y);
    static Node* rotateLeft(Node* x);
    static Node* minNode(Node* node);

    int   keyCompare(const OrderRecord& a, const OrderRecord& b) const;
    Node* balanceNode(Node* node);
    Node* insertNode(Node* node, const OrderRecord& rec);
    Node* removeNode(Node* node, const OrderRecord& rec, bool& removed);
    Node* searchNode(Node* node, const std::string& licenseNumber, const std::string& date) const;
    void  freeNode(Node* node);
    void  inorderTraversal(Node* node, std::vector<Node*>& result) const;
    void  reverseInorderTraversal(Node* node, std::vector<Node*>& result) const;
    void  printTreeRecursive(Node* node, std::vector<const char*>& stems, char childType) const;
};

#endif // AVL_TREE_H
