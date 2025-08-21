#ifndef AVL_TREE_H
#define AVL_TREE_H

#include <string>
#include <vector>

// Key used in the AVL tree.  It consists of a person's full name and
// phone number.  Two keys are compared lexicographically by name and then
// by phone number.
struct PersonKey {
    std::string fullName;
    int         phoneNumber;

    bool operator<(const PersonKey& other) const {
        if (fullName != other.fullName)
            return fullName < other.fullName;
        return phoneNumber < other.phoneNumber;
    }

    bool operator==(const PersonKey& other) const {
        return fullName == other.fullName && phoneNumber == other.phoneNumber;
    }
};

// Object oriented AVL tree implementation.  All tree operations are
// provided as member functions of the AVLTree class.
class AVLTree {
public:
    // Node of the tree.  Exposed so callers can inspect search results.
    struct Node {
        PersonKey        key;        // composite key
        int              height;     // height of the subtree
        Node*            left;       // left child
        Node*            right;      // right child
        std::vector<int> lineNumbers;// lines in the input file

        Node(const PersonKey& k, int line);
    };

    AVLTree();
    ~AVLTree();

    void insert(const PersonKey& key, int lineNumber);
    bool remove(const PersonKey& key);
    Node* search(const PersonKey& key) const;

    std::vector<Node*> inorderNodes() const;
    std::vector<Node*> reverseInorderNodes() const;

    bool removeLine(const PersonKey& key, int lineNumber);
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

    int   keyCompare(const PersonKey& a, const PersonKey& b) const;
    Node* balanceNode(Node* node);
    Node* insertNode(Node* node, const PersonKey& key, int lineNumber);
    Node* removeNode(Node* node, const PersonKey& key, bool& removed);
    Node* searchNode(Node* node, const PersonKey& key) const;
    void  freeNode(Node* node);
    void  inorderTraversal(Node* node, std::vector<Node*>& result) const;
    void  reverseInorderTraversal(Node* node, std::vector<Node*>& result) const;
    void  printTreeRecursive(Node* node, std::vector<const char*>& stems, char childType) const;
};

#endif // AVL_TREE_H
