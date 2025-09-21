#include "hashtable.hpp"

#include <cstdint>
#include <fstream>
#include <functional>
#include <iostream>
#include <utility>

Cell::Cell()
    : occupied(false), key(), index(0) {}

HashTable::HashTable(std::size_t initialSize, double maxLoad)
    : m_size(initialSize),
      m_count(0),
      m_initialSize(initialSize),
      m_maxLoadFactor(maxLoad),
      m_minLoadFactor(0.25),
      table(new Cell[initialSize]) {}

HashTable::HashTable(HashTable&& other) noexcept
    : m_size(other.m_size),
      m_count(other.m_count),
      m_initialSize(other.m_initialSize),
      m_maxLoadFactor(other.m_maxLoadFactor),
      m_minLoadFactor(other.m_minLoadFactor),
      table(other.table) {
    other.table = nullptr;
    other.m_size = other.m_count = other.m_initialSize = 0;
    other.m_maxLoadFactor = other.m_minLoadFactor = 0.0;
}

HashTable& HashTable::operator=(HashTable&& other) noexcept {
    if (this != &other) {
        delete[] table;
        m_size = other.m_size;
        m_count = other.m_count;
        m_initialSize = other.m_initialSize;
        m_maxLoadFactor = other.m_maxLoadFactor;
        m_minLoadFactor = other.m_minLoadFactor;
        table = other.table;
        other.table = nullptr;
        other.m_size = other.m_count = other.m_initialSize = 0;
        other.m_maxLoadFactor = other.m_minLoadFactor = 0.0;
    }
    return *this;
}

HashTable::~HashTable() {
    delete[] table;
}

bool HashTable::insert(const std::string& key, std::size_t listIndex) {
    std::size_t base = hashPrimary(key);

    for (std::size_t i = 0; i < m_size; ++i) {
        std::size_t idx = hashSecondary(base, key, i);
        if (!table[idx].occupied) {
            table[idx].occupied = true;
            table[idx].key = key;
            table[idx].index = listIndex;
            ++m_count;
            if (static_cast<double>(m_count) / m_size > m_maxLoadFactor) {
                rehash(m_size * 2);
            }
            return true;
        }
        if (table[idx].key == key) {
            return false;
        }
    }

    rehash(m_size * 2);
    return insert(key, listIndex);
}

bool HashTable::remove(const std::string& key, std::size_t& removedIndex) {
    int steps = 0;
    std::size_t slot = find_slot(key, &steps);
    if (slot == m_size) return false;

    removedIndex = table[slot].index;
    table[slot].occupied = false;
    table[slot].key.clear();
    table[slot].index = 0;
    --m_count;

    std::size_t curr = (slot + 1) % m_size;
    while (table[curr].occupied) {
        std::string keyToReinsert = std::move(table[curr].key);
        std::size_t indexToReinsert = table[curr].index;
        table[curr].occupied = false;
        table[curr].index = 0;
        --m_count;
        insert(keyToReinsert, indexToReinsert);
        curr = (curr + 1) % m_size;
    }

    if (m_size > m_initialSize &&
        static_cast<double>(m_count) / m_size < m_minLoadFactor) {
        rehash(m_size / 2);
    }
    return true;
}

bool HashTable::search(const std::string& key, std::size_t& listIndex, int& steps) const {
    int localSteps = 0;
    std::size_t slot = find_slot(key, &localSteps);
    steps = localSteps;
    if (slot == m_size) return false;
    listIndex = table[slot].index;
    return true;
}

bool HashTable::update_index(const std::string& key, std::size_t newIndex) {
    int steps = 0;
    std::size_t slot = find_slot(key, &steps);
    if (slot == m_size) return false;
    table[slot].index = newIndex;
    return true;
}

bool HashTable::contains(const std::string& key) const {
    int steps = 0;
    return find_slot(key, &steps) != m_size;
}

void HashTable::clear() {
    delete[] table;
    table = new Cell[m_size];
    m_count = 0;
}

void HashTable::print(std::ostream& out) const {
    out << "Idx | Status   | Key | ListIndex\n";
    for (std::size_t i = 0; i < m_size; ++i) {
        out << i << "   | "
            << (table[i].occupied ? "OCCUPIED" : "FREE    ");
        if (table[i].occupied) {
            out << " | " << table[i].key << " | " << table[i].index;
        }
        out << "\n";
    }
}

void HashTable::saveToFile(const std::string& filename) const {
    std::ofstream ofs(filename);
    print(ofs);
}

std::size_t HashTable::hashPrimary(const std::string& key) const {
    static constexpr std::uint64_t MUL = 11400714819323198485ULL;
    static std::hash<std::string> hasher;
    std::uint64_t k = hasher(key);
    std::uint64_t h = k * MUL;
    std::size_t idx = static_cast<std::size_t>(h % m_size);
    std::cout << "[Hash] primary(\"" << key << "\") = " << idx << "\n";
    return idx;
}

std::size_t HashTable::hashSecondary(std::size_t base, const std::string& key, std::size_t iteration) const {
    std::size_t idx = (base + iteration) % m_size;
    if (iteration > 0) {
        std::cout << "[Hash] secondary(\"" << key
                  << "\", iter=" << iteration
                  << ") = " << idx << "\n";
    }
    return idx;
}

std::size_t HashTable::find_slot(const std::string& key, int* steps) const {
    std::size_t base = hashPrimary(key);

    for (std::size_t i = 0; i < m_size; ++i) {
        if (steps) *steps = static_cast<int>(i + 1);
        std::size_t idx = hashSecondary(base, key, i);
        if (!table[idx].occupied) return m_size;
        if (table[idx].key == key) return idx;
    }
    return m_size;
}

void HashTable::rehash(std::size_t newSize) {
    Cell* oldTable = table;
    std::size_t oldSize = m_size;

    table = new Cell[newSize];
    m_size = newSize;
    m_count = 0;

    for (std::size_t i = 0; i < oldSize; ++i) {
        if (oldTable[i].occupied) {
            insert(oldTable[i].key, oldTable[i].index);
        }
    }
    delete[] oldTable;
}
