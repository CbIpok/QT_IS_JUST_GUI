#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <limits>
#include "hashtable.hpp"

// trim whitespace
static void trim(std::string& s) {
    const char* ws = " \t\r\n";
    auto b = s.find_first_not_of(ws);
    if (b == std::string::npos) { s.clear(); return; }
    auto e = s.find_last_not_of(ws);
    s = s.substr(b, e - b + 1);
}

void printMenu() {
    std::cout << "\n=== Operations: ===\n"
        << "1. Create table from file\n"
        << "2. Add element\n"
        << "3. Remove element (full match)\n"
        << "4. Find element\n"
        << "5. Clear table\n"
        << "6. Save table to file\n"
        << "7. Initialize table to given size\n"
        << "8. Delete table completely\n"
        << "9. Print table to console\n"
        << "0. Exit\n"
        << "Select operation: ";
}

int main3() {
    size_t initialSize;
    std::cout << "Enter initial hash table size: ";
    if (!(std::cin >> initialSize) || initialSize == 0) {
        std::cerr << "Invalid size.\n";
        return 1;
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    HashTable* ht = new HashTable(initialSize);
    bool running = true;

    auto checkTbl = [&]() {
        if (!ht) { std::cout << "Table not initialized. Use option 7.\n"; return false; }
        return true;
        };

    while (running) {
        printMenu();
        int choice; std::cin >> choice;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        switch (choice) {
        case 1: { // load
            if (!checkTbl()) break;
            std::cout << "Input filename: ";
            std::string fname; std::getline(std::cin, fname);
            std::cout << "Max non-empty lines (0=all): ";
            int maxL; std::cin >> maxL;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::ifstream ifs(fname);
            if (!ifs) { std::cerr << "Cannot open " << fname << "\n"; break; }
            ht->clear();
            int total = 0, used = 0;
            std::string line;
            while (std::getline(ifs, line)) {
                ++total;
                if (line.empty()) continue;
                if (++used > maxL && maxL > 0) break;

                std::stringstream ss(line);
                std::string fio, street, phoneStr, appStr;
                std::getline(ss, fio, ';');
                std::getline(ss, street, ';');
                std::getline(ss, phoneStr, ';');
                std::getline(ss, appStr);
                trim(fio); trim(street);
                trim(phoneStr); trim(appStr);

                Record rec;
                rec.fio = fio;
                rec.street = street;
                try { rec.phoneNumber = std::stoll(phoneStr); }
                catch (...) { rec.phoneNumber = 0; }
                try { rec.applicationNumber = std::stoi(appStr); }
                catch (...) { rec.applicationNumber = 0; }
                rec.originalLine = used;
                ht->insert(rec);
            }
            std::cout << "Loaded " << used << " of " << total << " lines.\n";
            break;
        }
        case 2: { // add
            if (!checkTbl()) break;
            Record rec;
            std::cout << "Name: ";           std::getline(std::cin, rec.fio);
            std::cout << "Street: ";         std::getline(std::cin, rec.street);
            std::cout << "Phone #: ";        std::cin >> rec.phoneNumber;
            std::cout << "Application #: ";  std::cin >> rec.applicationNumber;
            rec.originalLine = -1;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            ht->insert(rec);
            break;
        }
        case 3: { // remove
            if (!checkTbl()) break;
            Record rec;
            std::cout << "Name: ";           std::getline(std::cin, rec.fio);
            std::cout << "Street: ";         std::getline(std::cin, rec.street);
            std::cout << "Phone #: ";        std::cin >> rec.phoneNumber;
            std::cout << "Application #: ";  std::cin >> rec.applicationNumber;
            rec.originalLine = -1;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            if (ht->remove(rec)) std::cout << "Removed.\n";
            else                 std::cout << "Not found/mismatch.\n";
            break;
        }
        case 4: { // find
            if (!checkTbl()) break;
            std::string fio; int app;
            std::cout << "Name: "; std::getline(std::cin, fio);
            std::cout << "Application #: "; std::cin >> app;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            size_t idx; int steps = 0;
            if (ht->search(fio, app, idx, steps))
                std::cout << "Found at idx=" << idx
                << ", line=" << ht->getOriginalLine(idx)
                << " (" << steps << " probes)\n";
            else
                std::cout << "Not found after " << steps << " probes.\n";
            break;
        }
        case 5: if (checkTbl()) ht->clear(); break;
        case 6: {
            if (!checkTbl()) break;
            std::cout << "Output filename: ";
            std::string out; std::getline(std::cin, out);
            ht->saveToFile(out);
            break;
        }
        case 7: {
            std::cout << "New table size: ";
            size_t sz; std::cin >> sz;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            delete ht; ht = new HashTable(sz);
            break;
        }
        case 8: delete ht; ht = nullptr; break;
        case 9: if (checkTbl()) ht->print(std::cout); break;
        case 0: running = false; break;
        default: std::cout << "Invalid option.\n";
        }
    }

    delete ht;
    return 0;
}
