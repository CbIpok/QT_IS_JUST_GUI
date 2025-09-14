#include "hashtable.hpp"
#include <functional>
#include <cstdint>
#include <iostream>
#include <fstream>

Cell::Cell() : occupied(false), key(""), index(0) {}

HashTable::HashTable(size_t initialSize, double maxLoad)
    : m_size(initialSize), m_count(0), m_initialSize(initialSize),
      m_maxLoadFactor(maxLoad), m_minLoadFactor(0.25), table(new Cell[initialSize]) {}

HashTable::HashTable(HashTable&& other) noexcept
    : m_size(other.m_size), m_count(other.m_count), m_initialSize(other.m_initialSize),
      m_maxLoadFactor(other.m_maxLoadFactor), m_minLoadFactor(other.m_minLoadFactor),
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

size_t HashTable::hashPrimary(const std::string& key) const {
    static constexpr uint64_t MUL = 11400714819323198485ULL;
    static std::hash<std::string> hasher;
    uint64_t k = hasher(key);
    uint64_t h = k * MUL;
    return h % m_size;
}

size_t HashTable::hashSecondary(size_t base, const std::string& key, size_t iteration) const {
    (void)key;
    return (base + iteration) % m_size;
}

void HashTable::rehash(size_t newSize) {
    Cell* oldTable = table;
    size_t oldSize = m_size;

    table = new Cell[newSize];
    m_size = newSize;
    m_count = 0;

    for (size_t i = 0; i < oldSize; ++i) {
        if (oldTable[i].occupied) {
            insert(oldTable[i].key, oldTable[i].index);
        }
    }
    delete[] oldTable;
}

bool HashTable::insert(const std::string& key, size_t index) {
    size_t base = hashPrimary(key);
    for (size_t i = 0; i < m_size; ++i) {
        size_t idx = hashSecondary(base, key, i);
        if (!table[idx].occupied) {
            table[idx].occupied = true;
            table[idx].key = key;
            table[idx].index = index;
            ++m_count;
            if ((double)m_count / m_size > m_maxLoadFactor) {
                rehash(m_size * 2);
            }
            return true;
        }
        if (table[idx].key == key) return false;
    }
    rehash(m_size * 2);
    return insert(key, index);
}

bool HashTable::get_index(const std::string& key, size_t& index, int& steps) const {
    size_t base = hashPrimary(key);
    for (size_t i = 0; i < m_size; ++i) {
        steps = static_cast<int>(i + 1);
        size_t idx = hashSecondary(base, key, i);
        if (!table[idx].occupied) return false;
        if (table[idx].key == key) {
            index = table[idx].index;
            return true;
        }
    }
    return false;
}

bool HashTable::remove(const std::string& key, size_t& index) {
    size_t base = hashPrimary(key);
    for (size_t i = 0; i < m_size; ++i) {
        size_t idx = hashSecondary(base, key, i);
        if (!table[idx].occupied) return false;
        if (table[idx].key == key) {
            index = table[idx].index;
            table[idx].occupied = false;
            --m_count;
            size_t curr = (idx + 1) % m_size;
            while (table[curr].occupied) {
                Cell tmp = table[curr];
                table[curr].occupied = false;
                --m_count;
                insert(tmp.key, tmp.index);
                curr = (curr + 1) % m_size;
            }
            if (m_size > m_initialSize && (double)m_count / m_size < m_minLoadFactor) {
                rehash(m_size / 2);
            }
            return true;
        }
    }
    return false;
}

bool HashTable::update_index(const std::string& key, size_t newIndex) {
    size_t base = hashPrimary(key);
    for (size_t i = 0; i < m_size; ++i) {
        size_t idx = hashSecondary(base, key, i);
        if (!table[idx].occupied) return false;
        if (table[idx].key == key) {
            table[idx].index = newIndex;
            return true;
        }
    }
    return false;
}

void HashTable::clear() {
    delete[] table;
    table = new Cell[m_size];
    m_count = 0;
}

void HashTable::print(std::ostream& out) const {
    out << "Idx | Status   | Key;Index\n";
    for (size_t i = 0; i < m_size; ++i) {
        out << i << "   | "
            << (table[i].occupied ? "OCCUPIED" : "FREE    ");
        if (table[i].occupied) {
            out << " | " << table[i].key << ";" << table[i].index;
        }
        out << "\n";
    }
}
