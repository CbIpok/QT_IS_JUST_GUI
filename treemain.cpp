#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "avl_tree.h"

// Parse a line "FullName PhoneNumber" into PersonKey
static bool parse_line(const std::string& line, PersonKey& key) {
    std::size_t pos = line.rfind(' ');
    if (pos == std::string::npos) return false;

    std::string namePart = line.substr(0, pos);
    std::string phonePart = line.substr(pos + 1);

    if (namePart.empty()) return false;

    try {
        key.phoneNumber = std::stoi(phonePart);
    }
    catch (...) {
        return false;
    }
    key.fullName = namePart;
    return true;
}

// Reads a single line from console and parses it into PersonKey
// Returns true if successful, false otherwise. Prints error messages if fails.
static bool read_person_key_from_console(PersonKey& key) {
    std::cin.ignore(10000, '\n');
    std::string line;
    if (!std::getline(std::cin, line)) {
        std::cout << "Invalid input.\n";
        return false;
    }

    if (!parse_line(line, key)) {
        std::cout << "Invalid input format.\n";
        return false;
    }

    return true;
}

// Reads a line from console and parses it into PersonKey and lineNumber.
// Format: "FullName PhoneNumber lineNumber"
static bool read_person_key_and_line_from_console(PersonKey& key, int& lineNumber) {
    std::cin.ignore(10000, '\n');
    std::string line;
    if (!std::getline(std::cin, line)) {
        std::cout << "Invalid input.\n";
        return false;
    }

    std::size_t pos = line.rfind(' ');
    if (pos == std::string::npos) {
        std::cout << "Invalid input format.\n";
        return false;
    }
    std::string lineNumberStr = line.substr(pos + 1);
    std::string rest = line.substr(0, pos);

    if (!parse_line(rest, key)) {
        std::cout << "Invalid input format.\n";
        return false;
    }

    try {
        lineNumber = std::stoi(lineNumberStr);
    }
    catch (...) {
        std::cout << "Invalid line number.\n";
        return false;
    }

    return true;
}

static void load_from_input_file(AVLTree& tree) {
    std::ifstream in("input.txt");
    if (!in.is_open()) {
        std::cout << "Warning: Cannot open 'input.txt'. Starting with empty tree.\n";
        return;
    }

    std::string line;
    int lineNumber = 0;
    while (std::getline(in, line)) {
        lineNumber++;
        if (line.empty()) continue;
        PersonKey key;
        if (parse_line(line, key)) {
            avl_insert(&tree, key, lineNumber);
        }
        else {
            std::cout << "Skipping invalid line " << lineNumber << ": " << line << "\n";
        }
    }
}

static void print_tree_inorder(const AVLTree& tree) {
    std::vector<AVLNode*> nodes = avl_inorder_nodes(&tree);
    for (std::size_t i = 0; i < nodes.size(); i++) {
        AVLNode* node = nodes[i];
        std::cout << node->key.fullName << " " << node->key.phoneNumber << " Lines:";
        for (std::size_t j = 0; j < node->lineNumbers.size(); j++) {
            std::cout << " " << node->lineNumbers[j];
        }
        std::cout << "\n";
    }
}

static void print_tree_reverse_inorder(const AVLTree& tree) {
    std::vector<AVLNode*> nodes = avl_reverse_inorder_nodes(&tree);
    for (std::size_t i = 0; i < nodes.size(); i++) {
        AVLNode* node = nodes[i];
        std::cout << node->key.fullName << " " << node->key.phoneNumber << " Lines:";
        for (std::size_t j = 0; j < node->lineNumbers.size(); j++) {
            std::cout << " " << node->lineNumbers[j];
        }
        std::cout << "\n";
    }
}

static void save_tree_reverse_inorder(const AVLTree& tree, const std::string& filename) {
    std::ofstream out(filename.c_str());
    if (!out.is_open()) {
        std::cout << "Cannot open '" << filename << "' for writing.\n";
        return;
    }
    std::vector<AVLNode*> nodes = avl_reverse_inorder_nodes(&tree);
    for (std::size_t i = 0; i < nodes.size(); i++) {
        AVLNode* node = nodes[i];
        out << node->key.fullName << " " << node->key.phoneNumber;
        for (std::size_t j = 0; j < node->lineNumbers.size(); j++) {
            out << " " << node->lineNumbers[j];
        }
        out << "\n";
    }
}

static void input_and_remove(AVLTree& tree) {
    std::cout << "Enter FullName and PhoneNumber to remove:\n";
    PersonKey key;
    if (!read_person_key_from_console(key)) return;

    if (avl_remove(&tree, key)) {
        std::cout << "Element removed.\n";
    }
    else {
        std::cout << "Element not found.\n";
    }
}

static void input_and_search(AVLTree& tree) {
    std::cout << "Enter FullName and PhoneNumber to search:\n";
    PersonKey key;
    if (!read_person_key_from_console(key)) return;

    AVLNode* node = avl_search(&tree, key);
    if (node) {
        std::cout << "Element found: " << node->key.fullName << " " << node->key.phoneNumber << " Lines:";
        for (std::size_t j = 0; j < node->lineNumbers.size(); j++) {
            std::cout << " " << node->lineNumbers[j];
        }
        std::cout << "\n";
    }
    else {
        std::cout << "Element not found.\n";
    }
}

static void input_and_insert_single(AVLTree& tree) {
    std::cout << "Enter FullName, PhoneNumber and LineNumber to insert:\n";
    PersonKey key;
    int ln;
    if (!read_person_key_and_line_from_console(key, ln)) return;

    avl_insert(&tree, key, ln);
    std::cout << "Element inserted.\n";
}

static void input_and_remove_line(AVLTree& tree) {
    std::cout << "Enter FullName, PhoneNumber and line number to remove:\n";
    PersonKey key;
    int ln;
    if (!read_person_key_and_line_from_console(key, ln)) return;

    bool res = avl_remove_line(&tree, key, ln);
    if (res) {
        std::cout << "Line removed for the given key.\n";
    }
    else {
        std::cout << "No line removed (either key or line not found).\n";
    }
}

int main2() {
    AVLTree tree;
    avl_init(&tree);

    // Load from input.txt
    load_from_input_file(tree);

    bool initialized = true;

    for (;;) {
        std::cout << "--------------- Menu ---------------\n"
            << "1. Insert (single element)\n"
            << "2. Remove key\n"
            << "3. Search by key\n"
            << "4. Print the tree (in-order)\n"
            << "5. Traverse the tree (reverse in-order)\n"
            << "6. Free memory and exit (and save to output.txt)\n"
            << "7. Remove single line from a key\n"
            << "8. Print tree\n"
            << "Enter your choice: ";

        int choice;
        if (!(std::cin >> choice)) {
            std::cout << "Invalid input. Try again.\n";
            std::cin.clear();
            std::string dummy; std::getline(std::cin, dummy);
            continue;
        }

        switch (choice) {
        case 1:
            if (!initialized) {
                std::cout << "Tree not initialized.\n";
                break;
            }
            input_and_insert_single(tree);
            break;

        case 2:
            if (!initialized) {
                std::cout << "Tree not initialized.\n";
                break;
            }
            input_and_remove(tree);
            break;

        case 3:
            if (!initialized) {
                std::cout << "Tree not initialized.\n";
                break;
            }
            input_and_search(tree);
            break;

        case 4:
            if (!initialized) {
                std::cout << "Tree not initialized.\n";
                break;
            }
            std::cout << "Tree (in-order):\n";
            print_tree_inorder(tree);
            break;

        case 5:
            if (!initialized) {
                std::cout << "Tree not initialized.\n";
                break;
            }
            std::cout << "Tree (reverse in-order):\n";
            print_tree_reverse_inorder(tree);
            break;

        case 6:
            if (initialized) {
                save_tree_reverse_inorder(tree, "output.txt");
                avl_free(&tree);
                initialized = false;
                std::cout << "Memory freed and data saved to 'output.txt'.\n";
            }
            std::cout << "Exiting.\n";
            return 0;

        case 7:
            if (!initialized) {
                std::cout << "Tree not initialized.\n";
                break;
            }
            input_and_remove_line(tree);
            break;
        case 8:
            if (!initialized) {
                std::cout << "Tree not initialized.\n";
                break;
            }
            std::cout << "Tree structure:\n";
            avl_print_tree(&tree);
            break;
        default:
            std::cout << "Invalid choice. Try again.\n";
            break;
        }

        std::cout << "\n";
    }

    return 0;
}
