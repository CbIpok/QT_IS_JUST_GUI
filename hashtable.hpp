#ifndef HASHTABLE_HPP
#define HASHTABLE_HPP

#include <string>
#include <ostream>
#include <vector>
#include "record3.hpp"

// Closed addressing hash table implemented in an object-oriented manner.
// Each bucket (Cell) stores a single record or is marked as free.
class HashTable {
public:
    explicit HashTable(size_t initialSize, double maxLoad = 0.75);
    ~HashTable() = default;

    HashTable(const HashTable& other) = default;
    HashTable& operator=(const HashTable& other) = default;
    HashTable(HashTable&& other) noexcept = default;
    HashTable& operator=(HashTable&& other) noexcept = default;

    bool insert(const Record& rec);
    bool remove(const Record& rec);
    bool search(const std::string& fio, int applicationNumber,
        size_t& out_index, int& steps) const;

    void clear();
    void print(std::ostream& out) const;
    void saveToFile(const std::string& filename) const;
    int  getOriginalLine(size_t index) const;

private:
    struct Cell {
        bool   occupied;
        Record data;
        Cell();
    };

    size_t m_size, m_count, m_initialSize;
    double m_maxLoadFactor, m_minLoadFactor;
    std::vector<Cell> table;

    std::string makeKey(const std::string& fio, int applicationNumber) const;
    size_t      hashPrimary(const std::string& key) const;
    size_t      hashSecondary(size_t base, const std::string& key, size_t iteration) const;
    void        rehash(size_t newSize);
};

#endif // HASHTABLE_HPP
