#ifndef AVL_TREE_H
#define AVL_TREE_H

#include <string>
#include <vector>

// Key for identifying a person inside the tree
struct PersonKey {
    std::string fullName;
    int         phoneNumber;
};

// Object oriented AVL tree implementation
class AVLTree {
public:
    // Node used internally by the tree. It is public so that callers may
    // inspect search results, but all manipulations are performed through
    // the class methods.
    struct Node {
        PersonKey key;
        int       height;
        Node*     left;
        Node*     right;
        std::vector<int> lineNumbers; // positions of records with this key
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

    static int height(Node* n);
    static int maxint(int a, int b);
    static void update_height(Node* n);
    static int get_balance(Node* n);
    static Node* rotate_right(Node* y);
    static Node* rotate_left(Node* x);
    static int key_compare(const PersonKey& a, const PersonKey& b);
    static Node* create_node(const PersonKey& key, int lineNumber);
    static Node* balance_node(Node* node);
    static Node* min_node(Node* node);
    static Node* insert_node(Node* node, const PersonKey& key, int lineNumber);
    static Node* remove_node(Node* node, const PersonKey& key, bool& removed);
    static Node* search_node(Node* node, const PersonKey& key);
    static void free_node(Node* node);
    static void inorder_traversal_nodes(Node* node, std::vector<Node*>& result);
    static void reverse_inorder_traversal_nodes(Node* node, std::vector<Node*>& result);
    static void print_tree_recursive(Node* node,
                                    std::vector<const char*>& stems,
                                    char childType);
};

#endif // AVL_TREE_H

