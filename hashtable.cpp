#include "hashtable.hpp"
#include <functional>
#include <iostream>
#include <fstream>
#include <cstdint>

// ---------------- HashTable::Cell ----------------

HashTable::Cell::Cell()
    : occupied(false), index(-1) {}

// ---------------- HashTable ----------------

HashTable::HashTable(size_t initialSize, DoublyLinkedArray<Record>& storage, double maxLoad)
    : m_size(initialSize),
      m_count(0),
      m_initialSize(initialSize),
      m_maxLoadFactor(maxLoad),
      m_minLoadFactor(0.25),
      table(new Cell[initialSize]),
      m_storage(storage) {}

HashTable::~HashTable() {
    delete[] table;
}

std::string HashTable::makeKey(const std::string& fio, int applicationNumber) const {
    return fio + "#" + std::to_string(applicationNumber);
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
        if (!oldTable[i].occupied) continue;
        auto* node = m_storage.at(oldTable[i].index);
        if (!node) continue;
        const Record& rec = node->value;
        std::string key = makeKey(rec.fio, rec.applicationNumber);
        size_t base = hashPrimary(key);
        for (size_t j = 0; j < m_size; ++j) {
            size_t idx = hashSecondary(base, key, j);
            if (!table[idx].occupied) {
                table[idx].index = oldTable[i].index;
                table[idx].occupied = true;
                ++m_count;
                break;
            }
        }
    }
    delete[] oldTable;
}

bool HashTable::insert(const Record& rec) {
    std::string key = makeKey(rec.fio, rec.applicationNumber);
    size_t      base = hashPrimary(key);

    for (size_t i = 0; i < m_size; ++i) {
        size_t idx = hashSecondary(base, key, i);
        if (!table[idx].occupied) {
            int index = m_storage.push_back(rec);
            table[idx].index = index;
            table[idx].occupied = true;
            ++m_count;
            if ((double)m_count / m_size > m_maxLoadFactor) {
                rehash(m_size * 2);
            }
            return true;
        }
        auto* node = m_storage.at(table[idx].index);
        if (node && node->value.fio == rec.fio &&
            node->value.applicationNumber == rec.applicationNumber)
        {
            return false;
        }
    }
    rehash(m_size * 2);
    return insert(rec);
}

bool HashTable::search(const std::string& fio, int applicationNumber,
    size_t& out_index, int& steps) const {
    std::string key = makeKey(fio, applicationNumber);
    size_t      base = hashPrimary(key);

    for (size_t i = 0; i < m_size; ++i) {
        steps = int(i + 1);
        size_t idx = hashSecondary(base, key, i);
        if (!table[idx].occupied) return false;
        auto* node = m_storage.at(table[idx].index);
        if (node && node->value.fio == fio &&
            node->value.applicationNumber == applicationNumber)
        {
            out_index = idx;
            return true;
        }
    }
    return false;
}

bool HashTable::get(const std::string& fio, int applicationNumber, Record& out) const {
    size_t idx; int steps = 0;
    if (!search(fio, applicationNumber, idx, steps)) return false;
    auto* node = m_storage.at(table[idx].index);
    if (!node) return false;
    out = node->value;
    return true;
}

bool HashTable::remove(const Record& rec) {
    size_t idx; int steps = 0;
    if (!search(rec.fio, rec.applicationNumber, idx, steps))
        return false;

    auto* node = m_storage.at(table[idx].index);
    if (!node) return false;
    const Record& found = node->value;
    if (found.street != rec.street ||
        found.phoneNumber != rec.phoneNumber)
    {
        return false;
    }

    m_storage.remove(table[idx].index);
    table[idx].occupied = false;
    table[idx].index = -1;
    --m_count;
    std::string key = makeKey(rec.fio, rec.applicationNumber);
    size_t      base = hashPrimary(key);
    size_t      prev = idx;

    for (size_t i = steps; i < m_size; ++i) {
        size_t curr = hashSecondary(base, key, i);
        if (!table[curr].occupied) break;

        auto* node2 = m_storage.at(table[curr].index);
        const Record& r2 = node2 ? node2->value : Record{"",0,"",0,0};
        size_t home = hashPrimary(makeKey(r2.fio, r2.applicationNumber));

        bool inRange;
        if (home <= curr)
            inRange = (home <= prev && prev < curr);
        else
            inRange = (home <= prev || prev < curr);

        if (!inRange) {
            table[prev].index = table[curr].index;
            table[prev].occupied = true;
            table[curr].occupied = false;
            table[curr].index = -1;
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
    for (size_t i = 0; i < m_size; ++i) {
        if (table[i].occupied) {
            m_storage.remove(table[i].index);
        }
    }
    delete[] table;
    table = new Cell[m_size];
    m_count = 0;
}

void HashTable::print(std::ostream& out) const {
    out << "Idx | Status   | Record (name;app;street;phone;line)\n";
    for (size_t i = 0; i < m_size; ++i) {
        out << i << "   | "
            << (table[i].occupied ? "OCCUPIED" : "FREE    ");
        if (table[i].occupied) {
            auto* node = m_storage.at(table[i].index);
            const Record& r = node->value;
            out << " | "
                << r.fio << ";"
                << r.applicationNumber << ";"
                << r.street << ";"
                << r.phoneNumber << ";"
                << r.originalLine;
        }
        out << "\n";
    }
}

void HashTable::saveToFile(const std::string& filename) const {
    std::ofstream ofs(filename);
    print(ofs);
}

int HashTable::getOriginalLine(size_t index) const {
    if (index < m_size && table[index].occupied) {
        auto* node = m_storage.at(table[index].index);
        if (node) return node->value.originalLine;
    }
    return -1;
}
