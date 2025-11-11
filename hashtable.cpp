#include "hashtable.hpp"
#include <cstdint>
#include <fstream>
#include <functional>
#include <iostream>
#include "record_utils.hpp"

// ---------------- HashTable::Cell ----------------

HashTable::Cell::Cell()
    : occupied(false),
    data{ "", "", "", "", 0, "", -1 }
{}

// ---------------- HashTable ----------------

HashTable::HashTable(size_t initialSize, double maxLoad)
    : m_size(initialSize),
    m_count(0),
    m_initialSize(initialSize),
    m_maxLoadFactor(maxLoad),
    m_minLoadFactor(0.25),
    table(new Cell[initialSize])
{}

HashTable::~HashTable() {
    delete[] table;
}

std::string HashTable::makeKey(const std::string& licenseNumber) const {
    return licenseNumber;
}

// -- primary hash with debug logging --
size_t HashTable::hashPrimary(const std::string& key) const {
    static constexpr uint64_t MUL = 11400714819323198485ULL;
    static std::hash<std::string> hasher;
    uint64_t k = hasher(key);
    uint64_t h = k * MUL;
    size_t idx = h % m_size;
    std::cout << "[Hash] primary(\"" << key << "\") = " << idx << "\n";
    return idx;
}

// -- secondary (linear) probing with debug logging --
size_t HashTable::hashSecondary(size_t base, const std::string& key, size_t iteration) const {
    size_t idx = (base + iteration) % m_size;
    if (iteration > 0) {
        std::cout << "[Hash] secondary(\"" << key
            << "\", iter=" << iteration
            << ") = " << idx << "\n";
    }
    return idx;
}

void HashTable::rehash(size_t newSize) {
    Cell* oldTable = table;
    size_t oldSize = m_size;

    table = new Cell[newSize];
    m_size = newSize;
    m_count = 0;

    for (size_t i = 0; i < oldSize; ++i) {
        if (oldTable[i].occupied) {
            insert(oldTable[i].data);
        }
    }
    delete[] oldTable;
}

bool HashTable::insert(const Record& rec) {
    std::string key = makeKey(rec.licenseNumber);
    size_t      base = hashPrimary(key);

    for (size_t i = 0; i < m_size; ++i) {
        size_t idx = hashSecondary(base, key, i);
        if (!table[idx].occupied) {
            table[idx].data = rec;
            table[idx].occupied = true;
            ++m_count;
            if ((double)m_count / m_size > m_maxLoadFactor) {
                rehash(m_size * 2);
            }
            return true;
        }
        if (table[idx].data.licenseNumber == rec.licenseNumber) {
            return false;
        }
    }
    rehash(m_size * 2);
    return insert(rec);
}

bool HashTable::search(const std::string& licenseNumber,
    size_t& out_index, int& steps) const {
    std::string key = makeKey(licenseNumber);
    size_t      base = hashPrimary(key);

    for (size_t i = 0; i < m_size; ++i) {
        steps = int(i + 1);
        size_t idx = hashSecondary(base, key, i);
        if (!table[idx].occupied) return false;
        if (table[idx].data.licenseNumber == licenseNumber) {
            out_index = idx;
            return true;
        }
    }
    return false;
}

bool HashTable::remove(const Record& rec) {
    size_t idx; int steps = 0;
    if (!search(rec.licenseNumber, idx, steps))
        return false;

    const Record& found = table[idx].data;
    if (found.fio != rec.fio ||
        found.carBrand != rec.carBrand ||
        found.pickupAddress != rec.pickupAddress ||
        found.costKopecks != rec.costKopecks ||
        found.date != rec.date)
    {
        return false;
    }

    table[idx].occupied = false;
    --m_count;
    std::string key = makeKey(rec.licenseNumber);
    size_t      base = hashPrimary(key);
    size_t      prev = idx;

    for (size_t i = steps; i < m_size; ++i) {
        size_t curr = hashSecondary(base, key, i);
        if (!table[curr].occupied) break;

        const Record& r2 = table[curr].data;
        size_t home = hashPrimary(makeKey(r2.licenseNumber));

        bool inRange;
        if (home <= curr)
            inRange = (home <= prev && prev < curr);
        else
            inRange = (home <= prev || prev < curr);

        if (!inRange) {
            table[prev].data = table[curr].data;
            table[prev].occupied = true;
            table[curr].occupied = false;
            prev = curr;
        }
    }

    if (m_size > m_initialSize &&
        (double)m_count / m_size < m_minLoadFactor)
    {
        rehash(m_size / 2);
    }
    return true;
}

void HashTable::clear() {
    delete[] table;
    table = new Cell[m_size];
    m_count = 0;
}

void HashTable::print(std::ostream& out) const {
    out << "Idx | Status   | Record (license;fio;brand;address;cost;date;line)\n";
    for (size_t i = 0; i < m_size; ++i) {
        out << i << "   | "
            << (table[i].occupied ? "OCCUPIED" : "FREE    ");
        if (table[i].occupied) {
            const Record& r = table[i].data;
            out << " | "
                << r.licenseNumber << ";"
                << r.fio << ";"
                << r.carBrand << ";"
                << r.pickupAddress << ";"
                << record::formatCost(r.costKopecks) << ";"
                << r.date << ";"
                << r.originalLine;
        }
        out << "\n";
    }
}

void HashTable::saveToFile(const std::string& filename) const {
    std::ofstream ofs(filename);
    print(ofs);
}

void HashTable::saveReport(const std::string& filename) const {
    if (filename.empty()) {
        return;
    }
    std::ofstream ofs(filename);
    if (!ofs) {
        return;
    }
    for (size_t i = 0; i < m_size; ++i) {
        if (table[i].occupied) {
            ofs << record::serializeRecord(table[i].data) << '\n';
        }
    }
}

int HashTable::getOriginalLine(size_t index) const {
    if (index < m_size && table[index].occupied)
        return table[index].data.originalLine;
    return -1;
}

