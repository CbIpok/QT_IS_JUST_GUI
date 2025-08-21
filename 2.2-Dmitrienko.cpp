#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <filesystem>
#include "record2.hpp"
#include "merge_sort.hpp"
#include "modnaminecraft.hpp"

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

    std::vector<Record> recs;
    recs.reserve(n);

    bool needSort = true;
    if (fs::exists(sortedFile)) {
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
            int count = 0;
            while (count < n && std::getline(in, line)) {
                ++count;
                std::istringstream iss(line);
                Record r;
                std::string field;

                // originalLine
                if (!std::getline(iss, field, ';')) break;
                r.originalLine = std::stoi(field);

                // full name: Last First Patronymic
                if (!std::getline(iss, field, ';')) break;
                {
                    std::istringstream ns(field);
                    ns >> r.lastName >> r.firstName >> r.patronymic;
                }

                // street
                if (!std::getline(iss, field, ';')) break;
                r.street = trim(field);

                // phone
                if (!std::getline(iss, field, ';')) break;
                r.phoneNumber = std::stol(field);

                // applicationNumber
                if (!std::getline(iss, field)) break;
                r.applicationNumber = std::stoi(field);

                recs.push_back(r);
            }
            if ((int)recs.size() != n) {
                std::cerr << "Warning: sorted file has " << recs.size()
                    << " records (expected " << n << "); re-sorting.\n";
                needSort = true;
                recs.clear();
            }
            else {
                std::cout << "Loaded " << n << " sorted records from cache.\n";
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
        std::cout << "Reading " << n << " records\n";
        while (lineNo < n && std::getline(in, line)) {
            ++lineNo;
            const char* buf = line.data();
            const char* end = buf + line.size();

            // parse fields as before
            const char* sep1 = buf;
            while (sep1 < end && *sep1 != ';') ++sep1;
            if (sep1 == end) continue;
            Record r;
            r.originalLine = lineNo;
            // name
            const char* p = buf;
            const char* sp1 = p;
            while (sp1 < sep1 && *sp1 != ' ') ++sp1;
            if (sp1 == sep1) continue;
            r.lastName.assign(p, sp1 - p);
            p = sp1 + 1;
            const char* sp2 = p;
            while (sp2 < sep1 && *sp2 != ' ') ++sp2;
            if (sp2 == sep1) continue;
            r.firstName.assign(p, sp2 - p);
            p = sp2 + 1;
            r.patronymic.assign(p, sep1 - p);

            // street
            const char* streetStart = sep1 + 1;
            const char* sep2 = streetStart;
            while (sep2 < end && *sep2 != ';') ++sep2;
            if (sep2 == end) continue;
            const char* st = streetStart;
            while (st < sep2 && isspace(*st)) ++st;
            const char* ed = sep2 - 1;
            while (ed > st && isspace(*ed)) --ed;
            r.street.assign(st, ed - st + 1);

            // phone
            const char* phoneStart = sep2 + 1;
            const char* sep3 = phoneStart;
            while (sep3 < end && *sep3 != ';') ++sep3;
            if (sep3 == end) continue;
            long phone = 0;
            for (const char* q = phoneStart; q < sep3; ++q)
                if (isdigit(*q)) phone = phone * 10 + (*q - '0');
            r.phoneNumber = phone;

            // application number
            const char* appStart = sep3 + 1;
            int app = 0;
            while (appStart < end && !isdigit(*appStart)) ++appStart;
            for (const char* q = appStart; q < end && isdigit(*q); ++q)
                app = app * 10 + (*q - '0');
            r.applicationNumber = app;

            recs.push_back(r);
            if (lineNo % (n / 10 ? n / 10 : 1) == 0 || lineNo == n)
                std::cout << "  parsed " << lineNo << "/" << n << "\n";
        }
        if ((int)recs.size() < n) {
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
            out << r.originalLine << ";"
                << r.lastName << " " << r.firstName << " " << r.patronymic << ";"
                << r.street << ";"
                << r.phoneNumber << ";"
                << r.applicationNumber << "\n";
        }
    }

    // Prepare keys for search
    std::vector<int> keys;
    keys.reserve(n);
    for (const auto& r : recs)
        keys.push_back(r.applicationNumber);

    // Binary search
    int searchKey;
    std::cout << "Enter application number to search: ";
    while (!(std::cin >> searchKey)) {
        std::cin.clear();
        std::cin.ignore(1 << 20, '\n');
        std::cout << "Enter an integer: ";
    }

    std::cout << "Starting binary search...\n";
    auto binRes = binarySearch(keys, searchKey);
    if (binRes.first >= 0) {
        std::cout << "Binary search: found original line "
            << recs[binRes.first].originalLine
            << " (pos " << binRes.first + 1 << "), steps "
            << binRes.second << "\n";
    }
    else {
        std::cout << "Binary search: not found, steps " << binRes.second << "\n";
    }

    // Linear search
    std::cout << "Starting linear search...\n";
    auto linRes = linearSearch(keys, searchKey);
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
