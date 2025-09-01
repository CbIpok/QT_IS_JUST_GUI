#include "hashtable.hpp"
#include <functional>
#include <cstdint>
#include <iostream>
#include <fstream>

// ---------------- Cell ----------------

Cell::Cell()
    : occupied(false),
    data({ "", "", "", -1 })
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

HashTable::HashTable(HashTable&& other) noexcept
    : m_size(other.m_size),
      m_count(other.m_count),
      m_initialSize(other.m_initialSize),
      m_maxLoadFactor(other.m_maxLoadFactor),
      m_minLoadFactor(other.m_minLoadFactor),
      table(other.table)
{
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

bool HashTable::insert(const DriverRecord& rec) {
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
        if (table[idx].data.licenseNumber == rec.licenseNumber)
        {
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
        if (table[idx].data.licenseNumber == licenseNumber)
        {
            out_index = idx;
            return true;
        }
    }
    return false;
}

bool HashTable::remove(const DriverRecord& rec) {
    size_t idx; int steps = 0;
    if (!search(rec.licenseNumber, idx, steps))
        return false;

    const DriverRecord& found = table[idx].data;
    if (found.fio != rec.fio ||
        found.carBrand != rec.carBrand)
    {
        return false;
    }

    table[idx].occupied = false;
    --m_count;

    size_t curr = (idx + 1) % m_size;
    while (table[curr].occupied) {
        DriverRecord tmp = table[curr].data;
        table[curr].occupied = false;
        --m_count;
        insert(tmp);
        curr = (curr + 1) % m_size;
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
    out << "Idx | Status   | Record (license;fio;brand;line)\n";
    for (size_t i = 0; i < m_size; ++i) {
        out << i << "   | "
            << (table[i].occupied ? "OCCUPIED" : "FREE    ");
        if (table[i].occupied) {
            const DriverRecord& r = table[i].data;
            out << " | "
                << r.licenseNumber << ";"
                << r.fio << ";"
                << r.carBrand << ";"
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
    if (index < m_size && table[index].occupied)
        return table[index].data.originalLine;
    return -1;
}
