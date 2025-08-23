#include <iostream>
#include <vector>
#include "DoublyLinkedList.hpp"

using namespace std;

void menu() {
    vector<DoublyLinkedList> lists;
    lists.reserve(10);

    int choice, listIndex, value;

    while (true) {
        cout << "\nMenu:\n";
        cout << "1. Create new list\n";
        cout << "2. Insert element (at end)\n";
        cout << "3. Remove all occurrences\n";
        cout << "4. Remove before value\n";
        cout << "5. Search for value\n";
        cout << "6. Print list\n";
        cout << "7. Reverse list\n";
        cout << "8. Free list\n";
        cout << "9. Generate new list based on largest primes less than elements\n";
        cout << "10. Exit\n";
        cout << "Enter your choice: ";
        if (!(cin >> choice)) return;

        switch (choice) {
        case 1:
            if (lists.size() < 10) {
                lists.emplace_back();
                cout << "List " << (lists.size() - 1) << " created.\n";
            }
            else {
                cout << "Maximum number of lists reached.\n";
            }
            break;

        case 2:
            cout << "List index: ";
            cin >> listIndex;
            if (listIndex >= 0 && listIndex < (int)lists.size()) {
                cout << "Value to insert: ";
                cin >> value;
                lists[listIndex].push_back(value);
            }
            else {
                cout << "Invalid list index.\n";
            }
            break;

        case 3:
            cout << "List index: ";
            cin >> listIndex;
            if (listIndex >= 0 && listIndex < (int)lists.size()) {
                cout << "Value to remove: ";
                cin >> value;
                lists[listIndex].remove_all(value);
            }
            else {
                cout << "Invalid list index.\n";
            }
            break;

        case 4:
            cout << "List index: ";
            cin >> listIndex;
            if (listIndex >= 0 && listIndex < (int)lists.size()) {
                cout << "Remove nodes before value: ";
                cin >> value;
                lists[listIndex].remove_before_value(value);
            }
            else {
                cout << "Invalid list index.\n";
            }
            break;

        case 5:
            cout << "List index: ";
            cin >> listIndex;
            if (listIndex >= 0 && listIndex < (int)lists.size()) {
                cout << "Value to search: ";
                cin >> value;
                cout << (lists[listIndex].contains(value) ? "Value found.\n" : "Value not found.\n");
            }
            else {
                cout << "Invalid list index.\n";
            }
            break;

        case 6:
            cout << "List index: ";
            cin >> listIndex;
            if (listIndex >= 0 && listIndex < (int)lists.size()) {
                lists[listIndex].print();
            }
            else {
                cout << "Invalid list index.\n";
            }
            break;

        case 7:
            cout << "List index to reverse: ";
            cin >> listIndex;
            if (listIndex >= 0 && listIndex < (int)lists.size()) {
                lists[listIndex].reverse();
                cout << "List " << listIndex << " reversed.\n";
            }
            else {
                cout << "Invalid list index.\n";
            }
            break;

        case 8:
            cout << "List index to free: ";
            cin >> listIndex;
            if (listIndex >= 0 && listIndex < (int)lists.size()) {
                lists[listIndex].clear();
                cout << "List " << listIndex << " freed.\n";
            }
            else {
                cout << "Invalid list index.\n";
            }
            break;

        case 9:
            if (lists.size() < 10) {
                cout << "Enter the index of the input list: ";
                cin >> listIndex;
                if (listIndex >= 0 && listIndex < (int)lists.size()) {
                    DoublyLinkedList generated = lists[listIndex].map_to_prev_prime();
                    lists.emplace_back(std::move(generated));
                    cout << "New list generated at index " << (lists.size() - 1) << ".\n";
                }
                else {
                    cout << "Invalid list index.\n";
                }
            }
            else {
                cout << "Maximum number of lists reached.\n";
            }
            break;

        case 10:
            cout << "Exiting program...\n";
            return;

        default:
            cout << "Invalid choice.\n";
            break;
        }
    }
}

int main1() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    menu();
    return 0;
}
