#pragma once
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include "hashtable.hpp"
#include "record_utils.hpp"

// trim whitespace
static void trim(std::string& s) {
    const char* ws = " \t\r\n";
    auto b = s.find_first_not_of(ws);
    if (b == std::string::npos) { s.clear(); return; }
    auto e = s.find_last_not_of(ws);
    s = s.substr(b, e - b + 1);
}

static bool readRecordInteractive(const std::string& action, Record& rec) {
    std::cout << action << "\n";
    std::string value;
    std::string error;

    std::cout << "Номер лицензии: ";
    std::getline(std::cin, value);
    trim(value);
    if (!record::validateLicenseNumber(value, error)) {
        std::cout << "Ошибка: " << error << "\n";
        return false;
    }
    rec.licenseNumber = value;

    std::cout << "ФИО: ";
    std::getline(std::cin, value);
    trim(value);
    if (!record::validateFio(value, error)) {
        std::cout << "Ошибка: " << error << "\n";
        return false;
    }
    rec.fio = value;

    std::cout << "Марка авто: ";
    std::getline(std::cin, value);
    trim(value);
    if (!record::validateCarBrand(value, error)) {
        std::cout << "Ошибка: " << error << "\n";
        return false;
    }
    rec.carBrand = value;

    std::cout << "Адрес подачи: ";
    std::getline(std::cin, value);
    trim(value);
    if (!record::validateAddress(value, error)) {
        std::cout << "Ошибка: " << error << "\n";
        return false;
    }
    rec.pickupAddress = value;

    std::cout << "Стоимость (формат 123,45): ";
    std::getline(std::cin, value);
    trim(value);
    int cost = 0;
    if (!record::validateCost(value, cost, error)) {
        std::cout << "Ошибка: " << error << "\n";
        return false;
    }
    rec.costKopecks = cost;

    std::cout << "Дата (DD Mon YYYY): ";
    std::getline(std::cin, value);
    trim(value);
    if (!record::validateDate(value, error)) {
        std::cout << "Ошибка: " << error << "\n";
        return false;
    }
    rec.date = value;
    rec.originalLine = -1;
    return true;
}

void printMenu() {
    std::cout << "\n=== Operations: ===\n"
        << "1. Create table from file\n"
        << "2. Add element\n"
        << "3. Remove element (full match)\n"
        << "4. Find element by license\n"
        << "5. Clear table\n"
        << "6. Save table dump\n"
        << "7. Initialize table to given size\n"
        << "8. Delete table completely\n"
        << "9. Print table to console\n"
        << "10. Save report to file\n"
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
            trim(fname);
            std::cout << "Max records to load (0=all): ";
            int maxL; std::cin >> maxL;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::ifstream ifs(fname);
            if (!ifs) { std::cerr << "Cannot open " << fname << "\n"; break; }
            ht->clear();
            int total = 0, used = 0;
            std::string line;
            while (std::getline(ifs, line)) {
                ++total;
                trim(line);
                if (line.empty()) continue;
                Record rec;
                std::string error;
                if (!record::parseRecordFromInput(line, total, rec, error)) {
                    std::cerr << "Строка " << total << ": " << error << "\n";
                    continue;
                }
                rec.originalLine = total;
                if (ht->insert(rec)) {
                    ++used;
                }
                if (maxL > 0 && used >= maxL) break;
            }
            std::cout << "Loaded " << used << " of " << total << " lines.\n";
            break;
        }
        case 2: { // add
            if (!checkTbl()) break;
            Record rec;
            if (readRecordInteractive("Добавление записи", rec)) {
                if (!ht->insert(rec)) {
                    std::cout << "Запись с таким номером лицензии уже существует.\n";
                }
            }
            break;
        }
        case 3: { // remove
            if (!checkTbl()) break;
            Record rec;
            if (readRecordInteractive("Удаление записи", rec)) {
                if (ht->remove(rec)) std::cout << "Removed.\n";
                else                 std::cout << "Not found/mismatch.\n";
            }
            break;
        }
        case 4: { // find
            if (!checkTbl()) break;
            std::string license;
            std::cout << "Номер лицензии: ";
            std::getline(std::cin, license);
            trim(license);
            std::string error;
            if (!record::validateLicenseNumber(license, error)) {
                std::cout << "Ошибка: " << error << "\n";
                break;
            }
            size_t idx; int steps = 0;
            if (ht->search(license, idx, steps)) {
                std::cout << "Found at idx=" << idx
                    << ", line=" << ht->getOriginalLine(idx)
                    << " (" << steps << " probes)\n";
            }
            else {
                std::cout << "Not found after " << steps << " probes.\n";
            }
            break;
        }
        case 5: if (checkTbl()) ht->clear(); break;
        case 6: {
            if (!checkTbl()) break;
            std::cout << "Output filename: ";
            std::string out; std::getline(std::cin, out);
            trim(out);
            if (out.empty()) {
                std::cout << "Filename cannot be empty.\n";
                break;
            }
            ht->saveToFile(out);
            break;
        }
        case 7: {
            std::cout << "New table size: ";
            size_t sz; std::cin >> sz;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            delete ht; ht = sz ? new HashTable(sz) : nullptr;
            break;
        }
        case 8: delete ht; ht = nullptr; break;
        case 9: if (checkTbl()) ht->print(std::cout); break;
        case 10: {
            if (!checkTbl()) break;
            std::cout << "Report filename: ";
            std::string out; std::getline(std::cin, out);
            trim(out);
            if (out.empty()) {
                std::cout << "Filename cannot be empty.\n";
                break;
            }
            ht->saveReport(out);
            std::cout << "Report saved.\n";
            break;
        }
        case 0: running = false; break;
        default: std::cout << "Invalid option.\n";
        }
    }

    delete ht;
    return 0;
}

