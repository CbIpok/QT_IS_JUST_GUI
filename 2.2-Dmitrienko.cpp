#pragma once
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>
#include "merge_sort.hpp"
#include "modnaminecraft.hpp"
#include "record_utils.hpp"

static std::string trim(const std::string& s) {
    size_t st = s.find_first_not_of(" \t\r\n"),
        ed = s.find_last_not_of(" \t\r\n");
    return st == std::string::npos ? "" : s.substr(st, ed - st + 1);
}

int main2() {
    const std::string inputFile = "input.txt";
    const std::string sortedFile = "sorted.txt";
    namespace fs = std::filesystem;

    // 1) Ask user for n
    int n;
    do {
        std::cout << "Enter n (10 ≤ n ≤ 1000000): ";
    } while (!(std::cin >> n) || n < 10 || n > 1000000);
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    std::vector<Record> recs;
    recs.reserve(n);

    bool needSort = true;
    if (fs::exists(sortedFile) && fs::exists(inputFile)) {
        auto inTime = fs::last_write_time(inputFile);
        auto sortedTime = fs::last_write_time(sortedFile);
        if (sortedTime >= inTime) {
            // Sorted cache is up to date: load from sortedFile
            needSort = false;
            std::ifstream in(sortedFile);
            if (!in) {
                std::cerr << "Error: cannot open " << sortedFile << "\n";
                return 1;
            }
            std::string line;
            while (std::getline(in, line) && static_cast<int>(recs.size()) < n) {
                Record r;
                std::string error;
                if (!record::parseRecordFromCache(line, r, error)) {
                    std::cerr << "Cache parse error: " << error << ". Rebuilding cache.\n";
                    needSort = true;
                    recs.clear();
                    break;
                }
                recs.push_back(r);
            }
            if (!needSort && static_cast<int>(recs.size()) == n) {
                std::cout << "Loaded " << n << " sorted records from cache.\n";
            }
            else {
                needSort = true;
                recs.clear();
            }
        }
    }

    if (needSort) {
        // Parse input file
        std::ifstream in(inputFile);
        if (!in) {
            std::cerr << "Error: cannot open " << inputFile << "\n";
            return 1;
        }
        std::string line;
        int lineNo = 0;
        std::cout << "Reading records...\n";
        while (std::getline(in, line)) {
            ++lineNo;
            std::string trimmed = trim(line);
            if (trimmed.empty()) continue;
            Record r;
            std::string error;
            if (!record::parseRecordFromInput(trimmed, lineNo, r, error)) {
                std::cerr << "Line " << lineNo << ": " << error << "\n";
                continue;
            }
            r.originalLine = lineNo;
            recs.push_back(r);
            if (static_cast<int>(recs.size()) == n) break;
            if (lineNo % (n / 10 ? n / 10 : 1) == 0) {
                std::cout << "  parsed " << recs.size() << "/" << n << " valid records\n";
            }
        }
        if (static_cast<int>(recs.size()) < n) {
            std::cerr << "Error: only " << recs.size()
                << " valid lines out of requested " << n << "\n";
            return 1;
        }

        std::cout << "Sorting " << n << " records...\n";
        mergeSort(recs.data(), 0, n - 1);
        std::cout << "Sorting complete.\n";

        // Write sorted cache
        std::ofstream out(sortedFile);
        if (!out) {
            std::cerr << "Error: cannot create " << sortedFile << "\n";
            return 1;
        }
        for (const auto& r : recs) {
            out << record::serializeRecord(r) << "\n";
        }
    }

    // Prepare keys for search (by cost)
    std::vector<int> keys;
    keys.reserve(n);
    for (const auto& r : recs)
        keys.push_back(r.costKopecks);

    // Binary search
    std::string costStr;
    std::cout << "Enter cost to search (format 123,45): ";
    std::getline(std::cin, costStr);
    costStr = trim(costStr);
    int searchCost = 0;
    std::string error;
    while (!record::validateCost(costStr, searchCost, error)) {
        std::cout << "Ошибка: " << error << "\nВведите стоимость повторно: ";
        std::getline(std::cin, costStr);
        costStr = trim(costStr);
    }

    std::cout << "Starting binary search...\n";
    auto binRes = binarySearch(keys, searchCost);
    if (binRes.first >= 0) {
        const Record& found = recs[binRes.first];
        std::cout << "Binary search: found original line "
            << found.originalLine
            << " (pos " << binRes.first + 1 << "), steps "
            << binRes.second << "\n";
        std::cout << "  License: " << found.licenseNumber << "\n"
            << "  FIO: " << found.fio << "\n"
            << "  Brand: " << found.carBrand << "\n"
            << "  Address: " << found.pickupAddress << "\n"
            << "  Cost: " << record::formatCost(found.costKopecks) << "\n"
            << "  Date: " << found.date << "\n";
    }
    else {
        std::cout << "Binary search: not found, steps " << binRes.second << "\n";
    }

    // Linear search
    std::cout << "Starting linear search...\n";
    auto linRes = linearSearch(keys, searchCost);
    if (!linRes.first.empty()) {
        std::cout << "Linear search: found at original lines";
        for (int idx : linRes.first)
            std::cout << " " << recs[idx].originalLine;
        std::cout << ", steps " << linRes.second << "\n";
    }
    else {
        std::cout << "Linear search: not found, steps " << linRes.second << "\n";
    }

    return 0;
}

